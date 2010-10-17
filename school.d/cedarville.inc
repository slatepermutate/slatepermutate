<?php
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
<h2>Cedarville-specific Instructions</h2>
<p>
  SlatePermutate can be a useful tool for scheduling your next semester at <a href="http://cedarville.edu/">Cedarville University</a>.
</p>
<ol>
  <li>Get in touch with your advisor during advising/reading recess.</li>
  <li>Look up each class your advisor specified on this course listing page</li>
  <li>Enter each class into a SlatePermutate schedule and add each section that is listed that you are willing to take.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule which would work with the sections you specified.</li>
  <li>Print out your preferred schedule by choosing "print" and selecting a schedule.</li>
  <li>Wait until it's your turn to register and grab your preferred sections before they fill up!</li>
</ol> <!--'-->
EOF;
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

  /* Current academic departments. Update as needed. */
  $departments = array('be','ba','ca','ed','eg','es','hg','id','ll','ms','mu','ns','ph','py','sm','sw');
  $basepath = "http://cedarville.edu/courses/schedule/";

  $season = strtolower($season);
  $tables = array();
  foreach($departments as $department)
    {
      $html = file_get_contents($basepath . $year . $season . '_' . $department . '_' . 'all.htm');
      if (!$html)
	continue;
      $tables[$department] = table_parse(cedarville_html_fix($html));
    }

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

	  foreach ($meetings as $meeting)
	    {
	      $section_letter = $section_parts['section'];
	      if ($meeting['type'] == 'LECT')
		/**
		 * \todo this might not make much sense.		 
		 */
		$section_letter = 'L' . $section_letter;
	      $semester->section_add($section_parts['department'], $section_parts['course'],
				     new Section($section_letter, $instructor,
						 $meeting['time_start'], $meeting['time_end'],
						 $meeting['days']));
	    }
	}
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
  return preg_replace('/&([^;]{5})/', '&amp;$1', $html);
}
