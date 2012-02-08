<?php /* -*- mode: php; -*- */
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
 *
 * All of the code for crawling umich.
 *
 * Potential startpoints:
 * - http://lsa.umich.edu/cg/cg_advsearch.aspx (HTML/curl-based)
 * - http://ro.umich.edu/schedule/ (harder HTML for semester guessing, one CSV download for entire semester -- <=4MB)
 *
 * A single download, the CSV option, is preferred to having to issue
 * a series of HTTP requests. Each HTTP request has a lot of latency
 * and overhead which a one-shot download doesn't.
 */

/**
 * \brief
 *   Retrieve the list of semesters umich has available for crawling.
 *
 * \todo
 *   Some error handling.
 *
 * \param $school
 *   The school handle for umich.
 * \param $semesters
 *   An array to which Semester objects should be appended, one for
 *   each potentially crawlable semester.
 * \return
 *   0 on success, 1 on failure.
 */
function umich_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  $uri = 'http://ro.umich.edu/schedule/';
  $cookies = array();

  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML(school_crawl_geturi($uri, $cookies, $school_crawl_log));
  $semesters_xpath = new DOMXPath($semesters_dom);

  foreach ($semesters_dom->getElementsByTagName('table') as $table)
    {
      $table_tr = NULL;
      foreach ($semesters_xpath->query('tr', $table) as $table_tr)
	break;
      if (empty($table_tr))
	{
	  school_crawl_logf($school_crawl_log, 5, "Unable to find first row in table which I suspect is a table holding all of the semesters I'm interested in. I will try any other tables in this page and hopefully find one with a row in it...");
	  continue;
	}

      $semester_columns = array(
	'name' => school_crawl_table_resolve_column($table_tr, 'Term'),
	'csv' => school_crawl_table_resolve_column($table_tr, '/[cC][sS][vV]/', 'school_crawl_table_resolve_column_regexcmp'),
      );
      foreach ($semester_columns as $semester_column_name => $semester_column)
	if ($semester_column === FALSE)
	  {
	    school_crawl_logf($school_crawl_log, 4, "Unable to resolve columns %s onto a column in a semester listing table. Skipping this table.",
			      $semester_column_name);
	    $semester_columns = NULL;
	    break;
	  }
      if (empty($semester_columns))
	continue;

      $first = TRUE;
      foreach ($semesters_xpath->query('tr', $table) as $table_tr)
	if ($first)
	  {
	    /* Skip row of <th/> or titles. */
	    $first = FALSE;
	    continue;
	  }
	else
	  {
	    $rownodes = school_crawl_table_rownodes($table_tr);
	    $semester_name = $rownodes->item($semester_columns['name']);
	    $semester_csv = $rownodes->item($semester_columns['csv']);

	    if (!preg_match('/^(.+) ([0-9]+)$/', $semester_name->textContent, $matches))
	      {
		school_crawl_logf($school_crawl_log, 4, "Unable to parse semester name `%s'. Skipping this semester.",
				  $semester_name->textContent);
		continue;
	      }

	    $semester = new Semester($matches[2], $matches[1]);
	    $a = NULL;
	    foreach ($semesters_xpath->query('descendant::a', $semester_csv) as $a)
	      if ($a->hasAttribute('href'))
		break;
	    if (empty($a) || !$a->hasAttribute('href'))
	      {
		school_crawl_logf($school_crawl_log, 4, "Unable to find <a/> element with an href attribute for a CSV link for the %s semester. Skipping this semester. (textContent of CSV column: %s).",
				  $semester, $semester_csv->textContent);
		continue;
	      }
	    /*
	     * Secretively communicate some metadata to
	     * umich_crawl_semester().
	     */
	    $semester->umich_csv_href = $a->getAttribute('href');
	    $semesters[] = $semester;
	  }
    }

  return 0;
}

/**
 * \brief
 *   Handle the crawling of one semester of umich.
 *
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 * \param $semester
 *   A Semester object to populate with courses from this semester.
 * \param $csv_href
 *   A link to a CSV file which will be downloaded and parsed.
 */
function umich_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  school_crawl_logf($school_crawl_log, 3, "Crawling %s.",
		    $semester);

  $cookies = array();
  $uri = $semester->umich_csv_href;

  /* parse into lines and then each row needs to be individually parsed */
  $csv = str_getcsv(school_crawl_geturi($uri, $cookies, $school_crawl_log), PHP_EOL);

  $fields = array(
		  'Term' => FALSE /* $semester->season_get() . ' ' . $semester->year_get() */,
		  'Session' => FALSE /* "Regular Academic Session", "First 7 Week Session", "Second 7 Week Session" <-- half-semester support? */,
		  'Acad Group' => FALSE /* long version of the department sorta, more general than the subject field */,
		  'Class Nbr' => FALSE /* section synonym */,
		  'Subject' => FALSE /* "Mathematics (MATH)" */,
		  'Catalog Nbr' => FALSE /* "201", unqualified course_id */,
		  'Section' => FALSE /* You still reading these comments? */,
		  'Course Title' => FALSE /* for your sake, I hope you aren't */,
		  'Component' => FALSE /* "LAB", "LEC", "REC" -- i.e., meeting_type(?) */,
		  'Codes' => FALSE /* "P  W", "P   ", "P R ", "PI  ", "A   ", "P RW" ??????? (reminds me of ``svn status''). If flag[3] = 'W', then the class has a meeting times */,
		  'M' => FALSE /* if a day is enabled, it is set to itself. I.e., $row['M'] = 'M' or $row['M'] = '' */,
		  'T' => FALSE,
		  'W' => FALSE,
		  'TH' => FALSE,
		  'F' => FALSE,
		  'S' => FALSE,
		  'SU' => FALSE /* OK, we'll have to add Sunday support someday ;-) */,
		  'Start Date' => FALSE /* yea! */,
		  'End Date' => FALSE /* "12/13/2011" */,
		  'Time' => FALSE /* "1230-130PM", "9-1030AM", "1130-1PM" */,
		  'Location' => FALSE,
		  'Instructor' => FALSE,
		  'Units' => FALSE /* As in credit hours */,
		  );
  $ignored_fields = array(
			  'Term' => TRUE,
			  'Session' => TRUE,
			  'Acad Group' => TRUE,
			  'Codes' => TRUE,
			  'SU' => TRUE,
			  );

  foreach (str_getcsv($csv[0]) as $col_num => $col_name)
    if (isset($fields[$col_name]))
      $fields[$col_name] = $col_num;
    else
      school_crawl_logf($school_crawl_log, 6, "We do not recognize the %s column in the CSV file for %s.",
			$col_name, $semester);

  foreach ($fields as $field => $col_num)
    if ($col_num === FALSE
	&& empty($ignored_field[$field]))
      {
	school_crawl_logf($school_crawl_log, 2, "Unable to find column %s in CSV for %s. Skipping this semester.",
			  $field, $semester);
	return 1;
      }

  /* remove the row with heading from the CSV dataset */
  unset($csv[0]);

  /* Now actually parse some data :-). */
  $row_accumulation = array('Instructor' => '');
  foreach ($csv as $row)
    {
      $row = str_getcsv($row);
      $synonym = trim($row[$fields['Class Nbr']]);
      $course_id = trim($row[$fields['Catalog Nbr']]);

      if (!preg_match(';\(([A-Z]+)\)$;', $row[$fields['Subject']], $matches))
	{
	  school_crawl_logf($school_crawl_log, 5, "Unable to parse department string `%s'. Skipping section/course (synonym=%s).",
			    $row[$fields['Subject']], $synonym);
	  continue;
	}
      $dept = $matches[1];

      /**
       * \todo
       *   umich stores sometimes ranges of credit hours for courses,
       *   formatted like "1.00-3.00". This is generally done for ARR
       *   courses, where there is negotiation between the faculty and
       *   the student on how the course is arranged. slate_permutate
       *   should have a concept of a range of credit hours, then when
       *   calculating credit hours for the user it can present the
       *   total as a range... not that hard, but still a task ;-).
       */
      $credit_hours = (float)$row[$fields['Units']];

      $days = '';
      foreach (array('M' => 'm', 'T' => 't', 'W' => 'w', 'TH' => 'h', 'F' => 'f', 'S' => 's')
	       as $field => $day)
	if (strlen(trim($row[$fields[$field]])))
	  $days .= $day;

      if (!preg_match(';^([0-9]+)-([0-9]+)([AP])M$;', $row[$fields['Time']], $matches))
	{
	  /*
	   * Some courses exist but only have sections which have ARR
	   * for their meeting times. I think this means sometimes
	   * that the student is to arrange the course meeting with
	   * the instructor, other times just that the course is
	   * planned but not scheduled yet. These courses should still
	   * show up in autocomplete even if they have no meeting
	   * times.
	   */

	  if ($row[$fields['Time']] != 'ARR')
	    /* Log an unanticipated Time value */
	    school_crawl_logf($school_crawl_log, 4, "Unable to parse meeting time: `%s'. Skipping section/meeting (synonym=%s).",
			      $row[$fields['Time']], $synonym);
	  /* ensure that the class is added nonetheless */
	  if ($semester->class_get($dept, $course_id) === NULL)
	    /**
	     * \todo
	     *   SP does credit hours by section, what about Courses
	     *   with no sections because they're these weird limbo
	     *   `ARR' courses but these limbo courses still have a
	     *   number of credit hours?
	     */
	    $semester->class_add(new Course($dept . '-' . $course_id, $row[$fields['Course Title']]));
	  continue;
	}
      $time_end = umich_crawl_time($matches[2], $matches[3]);
      $time_start = umich_crawl_time($matches[1], FALSE, $time_end);
      /* umich defines course_slots by meeting_type. */
      $meeting_type = school_crawl_meeting_type(trim($row[$fields['Component']]));

      /*
       * Some information is only presented in the first row in a
       * listing of courses. Perform some accumulation here.
       */
      foreach (array('Instructor') as $key)
	if (strlen($curr_value = trim($row[$fields[$key]])))
	  $row_accumulation[$key] = $curr_value;

      $semester->section_meeting_add($dept, $course_id, trim($row[$fields['Course Title']]),
				     trim($row[$fields['Section']]), $synonym,
				     new SectionMeeting($days, $time_start, $time_end,
							trim($row[$fields['Location']]),
							$meeting_type,
							$row_accumulation['Instructor']),
				     $meeting_type,
				     $credit_hours);
    }
}

/**
 * \brief
 *   Try to turn a umich-formatted time into something usable.
 *
 * \param $raw
 *   The raw input.
 * \param $xm
 *   FALSE or, if PM or AM was specified, 'P' for PM and 'A' for AM.
 * \param $before
 *   A time of day before which this time must be. Used generally for
 *   the start time of a class. The end time of a class must be parsed
 *   first so that the result of that calculation may be passed as the
 *   $before value.
 */
function umich_crawl_time($raw, $xm = FALSE, $before = '2400')
{
  $h = $raw;
  $m = '00';
  if (strlen($raw) > 2)
    {
      $h = substr($raw, 0, strlen($raw) - 2);
      $m = substr($raw, strlen($raw) - 2);
    }

  $before_h = substr($before, 0, 2);
  $before_m = substr($before, 2);

  if ($xm === FALSE)
    {
      /* if the time could feasibly be in the afternoon, assume it is: */
      if (($h + 12) * 60 + $m < $before_h * 60 + $before_m)
	$xm = 'P';
      else
	$xm = 'A';
    }

  if (!strcmp($xm, 'P') && $h < 12)
    $h += 12;

  return sprintf('%02d%02d', $h, $m);
}
