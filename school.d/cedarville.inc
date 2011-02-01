<?php
/*
 * Copyright 2010 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
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

function cedarville_info()
{
  return array('name' => 'Cedarville University',
	       'url' => 'http://cedarville.edu/',
	       'domains' => array(
				  'cedarville.edu',
				  ),
	       'student_address' => 'Cedarville Student',
	       );
}

function cedarville_instructions_html()
{
  return <<<EOF
<h2>Instructions</h2>
<p>
  SlatePermutate can be a useful tool for scheduling your next semester at <a href="http://cedarville.edu/" rel="external">Cedarville University</a>.
</p>
<ol>
  <li>Enter the course ID, such as PHYS-1020, in the Class ID blank. You will see a list of auto-suggestions.</li>
  <li><strong>You must click on the auto-suggested item</strong> to automatically add all sections of the class.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule.</li>
  <li><strong>Schedule a meeting with your advisor to review your schedule.</strong></li>
  <li>When it's time to register, check the "Show Synonyms" box on your schedule and enter your course synonyms into the registration interface.</li>
</ol> <!--'-->
EOF;
}

/**
 * \brief
 *   Get a list of default classes (with sections (with meeting
 *   times)) for Cedarville students.
 *
 * \return
 *   An array of Course objects.
 */

function cedarville_default_classes()
{
  $chapel = new Course('Chapel');
  $chapel->section_add(new Section('_', array(new SectionMeeting('mtwhf', '1000', '1045', '', 'chapel')),
				   '', '_'));

  return array($chapel);
}


/**
 * \brief
 *   Parse given html into an array, first row is row headers
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
function cedarville_crawl($semester, $verbosity = 1)
{  

  $season = strtolower(substr($semester->season_get(), 0, 2));
  $year = $semester->year_get();
  $season_string = $year . $season;

  $basepath = 'http://cedarville.edu/courses/schedule/';

  if ($verbosity)
    echo "cedarville_crawl(): Beginning crawl of Cedarville:\n";

  if ($verbosity > 1)
    echo "cedarville_crawl(): Determining list of departments.\n";
  /*
   * We need two passes because the first department's code name is
   * not accessible available in the first pageload.
   */
  $departments = array();
  if (cedarville_crawl_departments_get($basepath . $year . $season . '_index.htm', $departments, $season_string))
    return 1;
  if (!count($departments))
    {
      echo "cedarville_crawl(): Unable to get a listing of departments.\n";
      return 1;
    }
  /* find the first department whose name we don't yet know */
  if (cedarville_crawl_departments_get($basepath . $year . $season . '_' . current(array_keys($departments)) . '_all.htm', $departments, $season_string))
    return 1;

  $tables = array();
  foreach ($departments as $department => $dept_name)
    {
      echo 'cedarville_crawl(): Crawling department ' . $department . ' (' . $dept_name . ")...\n";
      $html = file_get_contents($basepath . $year . $season . '_' . $department . '_' . 'all.htm');
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
	      error_log('Error parsing section_id. Given `' . $course_table[1] . '\', interpreted as `'
			. implode('-', $section_parts) . '\'. Skipping.');
	      continue;
	    }

	  $instructor = $course_table[3];

	  /*
	   * Each course may have multiple meeting times associated
	   * with it at Cedarville. We are not sure how to handle this
	   * quite, because different class sections may be tied with
	   * different lab meetings and stuff...
	   */
	  $meetings_str = $course_table[6];
	  if (strpos($meetings_str, 'TBA') !== FALSE)
	    {
	      if ($verbosity > 1)
		error_log('Skipping ' . implode('-', $section_parts) . ' because its meeting time info has `TBA\' in it.');
	      continue;
	    }
	  $meetings = array();
	  $meeting_multiple_types = array();
	  while (strlen($meetings_str) > 5)
	    {
	      if (!preg_match(';^ ([A-Z]+) +([A-Z]+[A-Z0-9]*) +([MTWRF]{1,5}) +([0-9:AP]+)-([0-9:AP]+);',
			      $meetings_str, $meeting_matches))
		{
		  if (preg_match(';^Dates:[^0-9]+([/0-9]{8})-([/0-9]{8});',
				 $meetings_str, $meeting_matches))
		    {
		      if ($verbosity > 4)
			error_log('Skipping some meeting data for '
				  . implode('-', $section_parts) . ' because it is a date range: `'
				  . $meeting_matches[0] . '\'');
		      $meetings_str = substr($meetings_str, strlen($meeting_matches[0]));
		      continue;
		    }

		  if ($verbosity > 0)
		    error_log('Error parsing meeting time. Given `' . $meetings_str . '\'. Skipping '
			      . implode('-', $section_parts));
		  break;
		}
	      /* prepare for parsing the next meeting time */
	      $meetings_str = substr($meetings_str, strlen($meeting_matches[0]));

	      if (isset($meetings[$meeting_matches[1]]))
		{
		  if ($verbosity > 0 && !isset($meeting_multiple_types[$meeting_matches[1]]))
		    {
		      error_log('Section ' . implode('-', $section_parts)
				. ' has multiple meeting times for meeting_type of '
				. $meeting_matches[1] . ' which my unflexible code which'
				. ' could be made more flexible doesn\'t yet support.'
				. ' Skipping the extra meeting times for this type of meeting.');
		      /* only give the above error once per type. */
		      $meeting_multiple_types[$meeting_matches[1]] = TRUE;
		    }
		  continue;
		}

	      $meetings[$meeting_matches[1]]
		= array('room' => $meeting_matches[2],
			'days' => school_crawl_days_str_format($meeting_matches[3]),
			'time_start' => school_crawl_time_format(strptime($meeting_matches[4] . 'M', '%I:%M%p')),
			'time_end' => school_crawl_time_format(strptime($meeting_matches[5] . 'M', '%I:%M%p')),
			'type' => $meeting_matches[1], 
			);
	    }

	  $section_meetings = array();
	  foreach ($meetings as $meeting)
	    {
	      $meeting_type = $meeting['type'];
	      if (isset($meeting_type_maps[$meeting_type]))
		$meeting_type = $meeting_type_maps[$meeting_type];

	      $section_meetings[] = new SectionMeeting($meeting['days'], $meeting['time_start'],
						       $meeting['time_end'], $meeting['room'],
						       $meeting_type);
	    }
	  $semester->section_add($section_parts['department'], $section_parts['course'],
				 new Section($section_parts['section'], $section_meetings,
					     $synonym, $instructor));
	}
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
function cedarville_crawl_departments_get($dept_url, array &$departments, $season_string)
{
  $html = file_get_contents($dept_url);
  $dept_dom = new DOMDocument();
  if (!$dept_dom->loadHTML(cedarville_html_fix($html)))
    {
      echo "cedarville_crawl(): Error determining list of available departments: Unable to parse HTML.\n";
      return 1;
    }
  $xpath = new DOMXPath($dept_dom);

  $dept_node_list = $xpath->query('/descendant::div[@id="contenttext"]/child::span[position()=1 or position()=2]/child::a');
  foreach ($dept_node_list as $dept_node)
    {
      $href = $dept_node->getAttribute('href');
      if (!preg_match('/^' . preg_quote($season_string, '/') . '_([a-z]+)_[a-z]+\.htm$/', $href, $matches))
	{
	  echo 'cedarvillege_crawl(): Error determining list of available departments: Unable to parse the department string out of href="' . $href . "\".\n";
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
