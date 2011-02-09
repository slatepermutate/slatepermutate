<?php
/*
 * Copyright 2011 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
 *
 * This file is a part of slate_permutate.
 *
 * slate_permutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * slate_permutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with slate_permutate.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \brief
 *   Crawl CCBCMD's registration stuffage.
 *
 * \param $semester
 *   The Semester object which I should populate.
 * \param $verbosity
 *   A scale from 0 to 10 determining how loud I should be.
 * \return
 *   1 on failure, 0 on success.
 */
function ccbcmd_crawl(array &$semesters, $verbosity = 1)
{
  $cookies = array();

  /*
   * It seems that http://ccbcmd.edu/schedule/sched.html is what we're
   * meant to start from. That's just a redirect to some other page
   * from which we get a listing of available semesters and choose
   * one.
   */
  $uri = 'http://ccbcmd.edu/schedule/sched.html';
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML(school_crawl_geturi($uri, $cookies, NULL, TRUE, 'ccbcmd_crawl_curlhook', $verbosity));
  $semesters_select_node = $semesters_dom->getElementById('term_input_id');
  if ($semesters_select_node === NULL)
    {
      fprintf(STDERR, "Could not get list of available semesters to choose from\n");
      return 1;
    }

  $semester_stage_uri = $uri;

  $semesters_form = school_crawl_element_ancestor($semesters_select_node, 'form');
  if ($semesters_form === NULL)
    {
      fprintf(STDERR, "Unable to find <form /> associated with semester.\n");
      return 1;
    }
  $semesters_post_save = school_crawl_form($semesters_form);

  foreach ($semesters_select_node->childNodes as $semesters_option_node)
    {
      $semester_text = $semesters_option_node->textContent;
      $semester_value = $semesters_option_node->getAttribute('value');
      if (empty($semester_value))
	/* skip the empty ``None'' semester */
	continue;

      if (stripos($semester_text, 'continuing') !== FALSE)
	/* skip the year-long semesters dedicated to continuing education */
	continue;

      $semester_text_parts = explode(' ', $semester_text);
      $semester_season = $semester_text_parts[0];
      $semester_year = $semester_text_parts[1];

      /* the college has two separate summer sessions, so distinguish between them */
      if (preg_match(';session ([0-9]+);i', $semester_text, $matches))
	$semester_season .= '_' . $matches[1];

      if ($verbosity)
	fprintf(STDERR, "Crawling semester %s:%s -> %s.\n", $semester_year, $semester_season, $semester_text);
      $semester = new Semester($semester_year, strtolower($semester_season));

  if ($verbosity > 1)
    fprintf(STDERR, "Found semester: %s=``%s''=``%s''.\n",
  	    $semester_value, $semester->id(), trim($semesters_option_node->textContent));
  /* load stored semester-page URI / form data */
  $semesters_post = $semesters_post_save;
  $uri = $semester_stage_uri;
  $semesters_post[$semesters_select_node->getAttribute('name')] = $semester_value;

  $subjects_dom = new DOMDocument();
  $uri = school_crawl_url($uri, $semesters_form->getAttribute('action'));
  $subjects_dom->loadHTML(school_crawl_geturi($uri, $cookies, $semesters_post, TRUE, 'ccbcmd_crawl_curlhook', $verbosity));

  $subjects_form_nodelist = $subjects_dom->getElementsByTagName('form');
  if (!$subjects_form_nodelist->length)
    {
      fprintf(STDERR, "Unable to find <form /> to submit for the subjects choosing page.\n");
      return 1;
    }
  $subjects_form_node = $subjects_form_nodelist->item(0);
  $subjects_post = school_crawl_form($subjects_form_node);

  $subjects_select_node = $subjects_dom->getElementById('subj_id');
  foreach ($subjects_select_node->childNodes as $subjects_option_node)
    if (!strcasecmp('all', trim($subjects_option_node->textContent)))
      $subjects_post[$subjects_select_node->getAttribute('name')][] = $subjects_option_node->getAttribute('value');

  $courses_dom = new DOMDocument();
  $uri = school_crawl_url($uri, $subjects_form_node->getAttribute('action'));
  $courses_dom->loadHTML(school_crawl_geturi($uri, $cookies, $subjects_post, TRUE, 'ccbcmd_crawl_curlhook', $verbosity));

  $courses_xpath = new DOMXPath($courses_dom);

  /* The second row of the table has all of the headers in it */
  $tr_header_nodelist = $courses_xpath->query('//table[@class="datadisplaytable" and position()=1]//tr[position()=2]');
  if (!$tr_header_nodelist->length)
    {
      fprintf(STDERR, "Unable to find the row of the course/section data table which gives us the mappings of column names onto columns.\n");
      return 1;
    }
  $tr_header_node = $tr_header_nodelist->item(0);

  $section_offsets = array(
			   'registration_number' => school_crawl_table_resolve_column($tr_header_node, 'CRN'),
			   'section_id' => school_crawl_table_resolve_column($tr_header_node, 'subj/crse/sec'),
			   /* there's a boolean column which says whether or not the course has any prerequisites/corequisites.... */
			   'credits' => school_crawl_table_resolve_column($tr_header_node, 'credhrs'),
			   /* there's a column for the number of contact hours, vs. credit hours */
			   'dates' => school_crawl_table_resolve_column($tr_header_node, 'sessiondates'),
			   );
  foreach (array('title', 'days', 'times', 'instructor', 'location') as $column_key)
    $section_offsets[$column_key] = school_crawl_table_resolve_column($tr_header_node, $column_key);
  /* there's also a column for ``session dates'' */

  /* error check and calculate the number of children that a node must have to be  */
  $max_offset = 0;
  foreach ($section_offsets as $name => $value)
    {
      if ($value === FALSE)
	{
	  fprintf(STDERR, "Unable to find column offset for `%s'.\n",
		  $name);
	  return 1;
	}
      else
	if ($verbosity > 6)
	  echo $name . ' -> ' . $value . PHP_EOL;

      $max_offset = max($max_offset, $value);
    }
      
  foreach ($courses_xpath->query('//table[@class="datadisplaytable" and position()=1]//tr') as $tr_node)
    {
      $children = school_crawl_table_rownodes($tr_node);
      if ($children->length < $max_offset)
	/*
	 * Skip this row because it doesn't have all of the columns we
	 * want and thus it can't be a row containing information
	 * about a section.
	 */
	continue;
      if (!strcmp($children->item($section_offsets['section_id'])->tagName, 'th'))
	/*
	 * We've hit one of the <tr/>s filled with <th/>s. Skip this one.
	 */
	continue;

      /*
       * There are some rows with the time set to TBA and with empty
       * section_id columns. Respond to this by skipping empty
       * section_id columns since there's no useful data in these
       * rows. We use strlen() < 3 because trim() doesn't take care of
       * &nbsp; :-/
       */
      $section_id = trim($children->item($section_offsets['section_id'])->textContent);
      if (strlen($section_id) < 3)
	continue;

      $section_id_parts = Section::parse($section_id);

      $registration_number = $children->item($section_offsets['registration_number'])->textContent;
      $instructor = $children->item($section_offsets['instructor'])->textContent;

      $section_meetings = array();
      {
	$time_range_text = $children->item($section_offsets['times'])->textContent;
	if (strpos($time_range_text, 'TBA') !== FALSE)
	  {
	    /*
	     * Add the section to the autocomplete list, just without
	     * any meeting info (i.e., $section_meetings is still
	     * empty now).
	     */
	    $semester->section_add($section_id_parts['department'], $section_id_parts['course'],
				   new Section($section_id_parts['section'], $section_meetings, $registration_number, $instructor));
	    continue;

	  }
	if (($dash_pos = strpos($time_range_text, '-')) === FALSE)
	  {
	    fprintf(STDERR, "Unable to understand course's time range format, cannot find dash: ``%s''.\n",
		    $time_range_text);
	    return 1;
	  }

	$time_start_text = substr($time_range_text, 0, $dash_pos);
	$time_start = strptime($time_start_text, '%I:%M %p');
	$time_end_text = substr($time_range_text, $dash_pos + 1);
	/*
	 * Make sure that _only_ one date range is specified to ensure
	 * data integrity. I.e., make sure that the college doesn't
	 * suddenly support multiple meeting times without our
	 * anticipating that and then cause us to have invalid
	 * data. ;-). --binki
	 */
	if (strpos($time_end_text, '-') !== FALSE)
	  {
	    fprintf(STDERR, "College seems to support multiple meeting times per semester which we don't know how to parse (even though slate_permutate itself can handle this situation): ``%s'' time_end_text: ``%s''.\n",
		    $time_range_text, $time_end_text);
	    return 1;
	  }
	$time_end = strptime($time_end_text, '%I:%M %p');
	if ($time_end === FALSE || $time_start === FALSE)
	  {
	    fprintf(STDERR, "Error parsing start or end time: start: ``%s'' end: ``%s''.\n",
		    $time_start_text, $time_end_text);
	    return 1;
	  }

	$days = school_crawl_days_str_format($children->item($section_offsets['days'])->textContent);

	$section_meetings[] = new SectionMeeting($days, school_crawl_time_format($time_start), school_crawl_time_format($time_end),
						 $children->item($section_offsets['location'])->textContent);

	/* check if a semester's date range should be increased */
	$section_dates = $children->item($section_offsets['dates'])->textContent;
	if (preg_match(';^([0-9]+)/([0-9]+)-([0-9]+)/([0-9]+)$;', $section_dates, $section_dates_matches))
	  {
	    $semester->time_start_set_test(mktime(0, 0, 0, $section_dates_matches[1], $section_dates_matches[2], $semester->year_get()));
	    $semester->time_end_set_test(  mktime(0, 0, 0, $section_dates_matches[3], $section_dates_matches[4], $semester->year_get()));
	  }
      }

      $semester->section_add($section_id_parts['department'], $section_id_parts['course'],
			     new Section($section_id_parts['section'], $section_meetings, $registration_number, $instructor));
    }

  $semesters[] = $semester;
    }

  return 0;
}

function ccbcmd_crawl_curlhook(&$curl)
{
  /*
   * OK, so this must be set to SSLv2 or SSLv3 because of how the
   * server's SSL junk is messed up. When curl is built against
   * gnutls, though, we can't use SSL2 since it doesn't support that
   * old of a protocol. So, we use 3 which works. Apparently, the
   * server can't handle gnutls's attempt to use TLS. Even openssl's
   * s_client command fails without manually specifying --ssl2 or
   * --ssl3. So, this must be a _really_ weird server setup...
   */
  curl_setopt($curl, CURLOPT_SSLVERSION, 3);
}
