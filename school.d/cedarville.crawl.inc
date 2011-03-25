<?php
/*
 * Copyright 2011 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
 *
 * This file is part of SlatePermutate.
 *
 * SlatePermutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SlatePermutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with SlatePermutate.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * \brief
 *   Crawler implementation for Cedarville University.
 */

/**
 * \brief
 *   Parse given html into an array, first row is row headers.
 *
 * \param $html
 *   HTML that PHP's DOM would willingly would eat.
 */
function table_parse($html)
{
  libxml_use_internal_errors(true); // Suppress warnings
  $arr = array();
  $dom = new DOMDocument;
  if(!$html)
    return NULL;

  $dom->loadHTML($html);
  $dom->preserveWhiteSpace = FALSE;
  $tables = $dom->getElementsByTagName('table');
  $rows = $tables->item(0)->getElementsByTagName('tr'); // Get first table on page 
  foreach ($rows as $rownum => $row) {
    $cols = $row->getElementsByTagName('td');
    foreach($cols as $colnum => $col){
      $arr[$rownum][$colnum] = $col->nodeValue;
    }
  }
  return $arr;
}

/** Crawls Cedarville course listings. $season is "fa" or "sp", year is 4-digit year */
function cedarville_crawl(array &$semesters, &$school_crawl_log)
{  
  $basepath = 'http://cedarville.edu/courses/schedule/';

  school_crawl_logf($school_crawl_log, 6, "Beginning crawl of Cedarville:");

  school_crawl_logf($school_crawl_log, 7, "Determining list of departments.");

  school_crawl_logf($school_crawl_log, 8, "Determining list of semesters.");
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML(file_get_contents($basepath));

  $content_div_dom = $semesters_dom->getElementById('contenttext');
  if (!$content_div_dom)
    {
      school_crawl_logf($school_crawl_log, 6, "Error finding location of the list of departments.");
      if (count($semesters))
	{
	  school_crawl_logf($school_crawl_log, 6, "Assuming that I got enough info anyways, returning successful code so that the few semesters I was able to crawl will be cached.");
	  return 0;
	}
      school_crawl_logf($school_crawl_log, 0, "Couldn't find any departments.");
      return 1;
    }
  $departments_xpath = new DOMXPath($semesters_dom);
  foreach ($departments_xpath->query('.//li/a') as $department_a_dom)
    {
      $semester_href = $department_a_dom->getAttribute('href');
      $semester_href_parts = split('_', $semester_href);

      $semester_name = $department_a_dom->textContent;
      if (stripos($semester_name, 'graduate') !== FALSE
	  || strpos($semester_href, 'index') === FALSE)
	/* cedarville has about 1 graduate course, lol */
	continue;
      $semester_name_parts = split(' ', $semester_name);

      $semester_year = $semester_name_parts[0];
      $semester_season = strtolower($semester_name_parts[1]);
      $semester_min_date_start = 0;
      $semester_max_date_end = 0;

      $semester = new Semester($semester_year, $semester_season);

      school_crawl_logf($school_crawl_log, 6, "Crawling semester: %s.",
			$semester_name);

  /*
   * We need two passes because the first department's code name is
   * not accessible available in the first pageload.
   */
  $departments = array();
  if (cedarville_crawl_departments_get($basepath . $semester_href, $departments, $semester_href_parts[0], $school_crawl_log))
    return 1;
  if (!count($departments))
    {
      school_crawl_logf($school_crawl_log, 6, "Unable to get a listing of departments.");
      if (count($semesters))
	{
	  school_crawl_logf($school_crawl_log, 6, "Assuming that I got enough info anyways, returning successful code so that the few semesters I was able to crawl will be cached.");
	  return 0;
	}
      school_crawl_logf($school_crawl_log, 0, "Unable to get listing of departments.");
      return 1;
    }
  /* find the first department whose name we don't yet know */
  if (cedarville_crawl_departments_get($basepath . $semester_href_parts[0] . '_' . current(array_keys($departments)) . '_all.htm', $departments, $semester_href_parts[0], $school_crawl_log))
    return 1;

  $tables = array();
  foreach ($departments as $department => $dept_name)
    {
      school_crawl_logf($school_crawl_log, 7, "Crawling department %s (%s).", $department, $dept_name);
      $html = file_get_contents($basepath . $semester_href_parts[0] . '_' . $department . '_' . 'all.htm');
      if (!$html)
	continue;
      $tables[$department] = table_parse(cedarville_html_fix($html));
    }

  $meeting_type_maps = array('LAB' => 'lab', 'LECT' => 'lecture');

  foreach ($tables as $dept_table)
    {
      /*
       * Discard the first row, which has the contents of the <th />
       * elements.
       */
      unset($dept_table[0]);

      foreach($dept_table as $course_table)
	{
	  /*
	   * format:
	   * 0: course synonym, an unsigned integer.
	   * 1: section spec, parsable by Section::parse().
	   * 2: friendly course title.
	   * 3: Instructor name.
	   * 4: Number of credit hours in decimal notation.
	   * 5: Fee.
	   * 6: Meeting time, explained below.
	   * 7: Cap.
	   * 8-10: Textbook link. Most rows only have column 8, not
	   *       all the way through 10. This information seems
	   *       quite useless.
	   *
	   * Section meeting time/place format:
	   *
	   * Confusing example: ' ILB  WI219   TR    08:30A-09:45A'
	   * Complete example plus lab: ' LEC  TYL203  MWF   08:00A-08:50A LAB  ENS118  TR    03:00P-04:30P'
	   *
	   * Appears to have format:
	   * <meeting_info>: <type> <room> <days> <time_start>-<time_end> <meeting_info>
	   *
	   * It appears tht <type> may be:
	   * LEC: normal lecture meeting.
	   * ONL: online course.
	   * ILB: ethan says a partially online course...?
	   * HYB: hybrid of...?
	   * FLD: field...?
	   * FE2: ?
	   * CLN: ?
	   * LAB: Lab
	   * LES: something for some PFMU/PLMU class?
	   */

	  $synonym = $course_table[0];
	  $section_parts = Section::parse($course_table[1]);
	  if (count($section_parts) < 3)
	    {
	      school_crawl_logf($school_crawl_log, 6, "Error parsing section_id. Given `%s'; interpreted as `%s'. Skipping.",
				$course_table[1], implode('-', $section_parts));
	      continue;
	    }

	  $instructor = $course_table[3];
          $title = $course_table[2];

	  /*
	   * Each course may have multiple meeting times associated
	   * with it at Cedarville. We are not sure how to handle this
	   * quite, because different class sections may be tied with
	   * different lab meetings and stuff...
	   */
	  $meetings_str = $course_table[6];
	  if (strpos($meetings_str, 'TBA') !== FALSE)
	    {
	      school_crawl_logf($school_crawl_log, 8, "Skipping %s because its meeting time info has `TBA' in it.", implode('-', $section_parts));
	      continue;
	    }
	  $meetings = array();
	  $meeting_multiple_types = array();
	  while (strlen($meetings_str) > 5)
	    {
	      $meeting_start_regex = ';^';
	      $meeting_base_regex = ' ([A-Z]+) +([A-Z]+[A-Z0-9]*) +([MTWRF]{1,5}) +([0-9:AP]+)-([0-9:AP]+)';
	      $meeting_date_regex = 'Dates:[^0-9]+([/0-9]{8})-([/0-9]{8})';
	      $meeting_end_regex = ';';
	      if (!preg_match($meeting_start_regex . $meeting_base_regex . $meeting_date_regex . $meeting_end_regex,
			      $meetings_str, $meeting_matches)
		  && !preg_match($meeting_start_regex . $meeting_base_regex . $meeting_end_regex,
				 $meetings_str, $meeting_matches))
		{
		  if (preg_match($meeting_start_regex . $meeting_date_regex . $meeting_end_regex,
				 $meetings_str, $meeting_matches))
		    {

		      school_crawl_logf($school_crawl_log, 8, "Skipping some meeting data for %s because it is a date range: `%s'.",
					implode('-', $section_parts), $meeting_matches[0]);
		      $meetings_str = substr($meetings_str, strlen($meeting_matches[0]));
		      continue;
		    }

		  school_crawl_logf($school_crawl_log, 6, "Error parsing meeting time. Given `%s'. Skipping %s.", $meetings_str, implode('-', $section_parts));
		  break;
		}
	      /* prepare for parsing the next meeting time */
	      $meetings_str = substr($meetings_str, strlen($meeting_matches[0]));

	      $days = school_crawl_days_str_format($meeting_matches[3]);
	      $time_start = school_crawl_time_format(strptime($meeting_matches[4] . 'M', '%I:%M%p'));
	      $time_end = school_crawl_time_format(strptime($meeting_matches[5] . 'M', '%I:%M%p'));
	      $room = $meeting_matches[2];

	      $type = $meeting_matches[1];
	      while (isset($meeting_type_maps[$type]))
		$type = $meeting_type_maps[$type];
	      $type = strtolower($type);

	      /* check for daterange information -- i.e., if the first regex successfully matched: */
	      if (count($meeting_matches) > 7)
		{
		  $date_start = school_crawl_mktime(strptime($meeting_matches[6], '%m/%d/%y'));
		  $date_end = school_crawl_mktime(strptime($meeting_matches[7], '%m/%d/%y'));
		  if (!empty($date_start) && !empty($date_end))
		    {
		      $semester->time_start_set_test($date_start);
		      $semester->time_end_set_test($date_end);
		    }
		}

	      $meetings[] = new SectionMeeting($days, $time_start, $time_end,
					       $room, $type);
	    }

	  $semester->section_add($section_parts['department'], $section_parts['course'],
				 new Section($section_parts['section'], $meetings,
					     $synonym, $instructor), $title);
	}
    }

  $semesters[] = $semester;
    }

  return 0;
}

/**
 * \brief
 *   Scan cedarville's course listing pages for departments.
 *
 * \return
 *   An associative array mapping department codes onto department
 *   friendly names.
 */
function cedarville_crawl_departments_get($dept_url, array &$departments, $season_string, $school_crawl_log)
{
  $html = file_get_contents($dept_url);
  $dept_dom = new DOMDocument();
  if (!$dept_dom->loadHTML(cedarville_html_fix($html)))
    {
      school_crawl_logf($school_crawl_log, 6, "Error determining list of available departments: Unable to parse HTML.");
      return 1;
    }
  $xpath = new DOMXPath($dept_dom);

  $dept_node_list = $xpath->query('/descendant::div[@id="contenttext"]/child::span[position()=1 or position()=2]/child::a');
  foreach ($dept_node_list as $dept_node)
    {
      $href = $dept_node->getAttribute('href');
      if (!preg_match('/^' . preg_quote($season_string, '/') . '_([a-z]+)_[a-z]+\.htm$/', $href, $matches))
	{
	  school_crawl_logf($school_crawl_log, 6, "cedarvillege_crawl(): Error determining list of available departments: Unable to parse the department string out of href=\"%s\".", $href);
	  return 1;
	}

      $dept = $matches[1];
      $departments[$dept] = $dept_node->textContent;
    }

  return 0;
}

/**
 * \brief
 *   Fix some incorrect usage of the HTML entity delimiter, the ampersand.
 */
function cedarville_html_fix($html)
{
  $html = preg_replace('/&&/', '&amp;&', $html);
  $html = preg_replace('/&([^;]{5})/', '&amp;$1', $html);
  $html = preg_replace('/ID="(LINKS|HERE)"/', '', $html);

  return $html;
}
