<?php
/*
 * Copyright 2010 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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
 *   Crawl's Calvin's registration course listing pages.
 *
 * \param $semesters
 *   An array to be filled with Semester objects which I should
 *   populate.
 * \param $school_crawl_log
 *   A school_crawl_log handle.
 */
function calvin_crawl(array &$semesters, &$school_crawl_log)
{
  /**
   * The first link we start at is the one from KV into WebAdvisor.
   *
   * 1. https://kvdata.calvin.edu/walive/WebAdvisor?CONSTITUENCY=WBST&type=P&pid=ST-WESTS12A&LASTTOKEN=NULL
   *    <body onload="javascript:getWindowHTML();">
   *
   *    Calls javascript:getWindowHTML(). This merely adds
   *    TOKENIDX=NULL to the query string, so we can skip this step
   *    and just have TOKENIDX=NULL.
   *
   * 2. https://kvdata.calvin.edu/walive/WebAdvisor?CONSTITUENCY=WBST&type=P&pid=ST-WESTS12A&LASTTOKEN=NULL&TOKENIDX=NULL
   *    <body onload="javascript:setWindowHTML('', '7699844013');">
   *
   *    In the above, the second argument to setWindowHTML() is
   *    random. Thus, we have to capture this value.
   */

  $cookies = array();

  $uri = 'https://kvdata.calvin.edu/walive/WebAdvisor?CONSTITUENCY=WBST&type=P&pid=ST-WESTS12A&LASTTOKEN=NULL';
  $departments_html = calvin_crawl_geturi($uri, $cookies, $school_crawl_log);

  $departments_dom = new DOMDocument();
  $departments_dom->loadHTML($departments_html);

  /*
   * Discover the available semesters
   */
  $semesters_select_nodes = $departments_dom->getElementById('VAR1')->childNodes;
  $semester_strs = array();
  foreach ($semesters_select_nodes as $semester_node)
    {
      if ($semester_node->tagName != 'option'
	  || !$semester_node->hasAttribute('value')
	  || !strlen($semester_node->getAttribute('value')))
	continue;
      $semester_strs[$semester_node->getAttribute('value')] =
	$semester_node->nodeValue;
    }
  $semester_strs = array_reverse($semester_strs, TRUE);

  $departments_select_nodes = $departments_dom->getElementById('LIST_VAR1_1')->childNodes;
  $departments = array();
  foreach ($departments_select_nodes as $dept_node)
    {
      if ($dept_node->tagName != 'option'
	  || !$dept_node->hasAttribute('value'))
	continue;
      $departments[$dept_node->getAttribute('value')] =
	$dept_node->nodeValue;
    }


  /*
   * get all of the different possible course levels... dynamically
   * rather than hardcodedly ;-).
   */
  $departments_select_nodes = $departments_dom->getElementById('LIST_VAR1_2')->childNodes;
  $course_levels = array();
  foreach ($departments_select_nodes as $courselevel_node)
    {
      if ($courselevel_node->tagName != 'option'
	  || !$courselevel_node->hasAttribute('value'))
	continue;
      $course_levels[] = $courselevel_node->getAttribute('value');
    }

  $return_url = dom_input_value($departments_dom, 'RETURN.URL');


  school_crawl_logf($school_crawl_log, 7, "Available semesters: %s.", implode($semester_strs, ', '));

  $semester_start_uri = $uri;

  $season_map = array(
		      'FA' => Semester::SEASON_FALL,
		      'IN' => 'interim',
		      'SP' => Semester::SEASON_SPRING,
		      'MA' => 'may',
		      /* I don't know if SU is a valid Calvin Smester ID or not */
		      'SU' => Semester::SEASON_SUMMER);
  foreach ($semester_strs as $semester_str => $semester_info)
    {
      if (empty($season_map[substr($semester_str, 3)]))
	{
	  school_crawl_logf($school_crawl_log, 6, "Warning: Unknown semester identification chars: %s. Skipping this semester.",
		  $semester_str);
	  continue;
	}
      $season = $season_map[substr($semester_str, 3)];
      $year_timespec = strptime(substr($semester_str, 0, 2), '%y');
      $year =  $year_timespec['tm_year'] + 1900;

      $semester = new Semester($year, $season);

      /* useful and necessary stats */
      $skipped_sections = array('incomplete meeting info' => 0, 'invalid meeting info format' => 0);

      $semester_start_min = 0;
      $semester_end_max = 0;

      $dept = '';
      $course_level = '';
      $uri = $semester_start_uri;

      school_crawl_logf($school_crawl_log, 6, "Crawling semester %s->%s.",
			$semester_str, $semester_info);

  /*
   * LIST.VAR<X>_<N>: <X> is the column, <N> is the row. There
   * are apparently a max of 5 rows (see the LIST.VAR<X>_MAX
   * below).
   *
   * Columns:
   * LIST.VAR1: department
   * LIST.VAR2: course_level
   * LIST.VAR3: IIRC, a course identifier, such as 156 from MATH-156
   * LIST.VAR4: I forget
   *
   */
  $form = array('VAR1' => $semester_str,
		'LIST.VAR1_1' => $dept,
		'LIST.VAR2_1' => $course_level,

		/*
		 * Other form items we're not querying but which need
		 * to be sent blankly.
		 */
		 'RETURN.URL' => $return_url,
		 'SUBMIT_OPTIONS' => '',
		 /*
		  * The submit button... its value="" key is
		  * apparently sent with the form... makes a
		  * little bit of sense I guess ;-).
		  */
		 /*'SUBMIT2' => 'SUBMIT',*/

		 'DATE.VAR1' => '',
		 'DATE.VAR2' => '',

		 'LIST.VAR1_CONTROLLER' => 'LIST.VAR1',
		 'LIST.VAR1_MEMBERS' => 'LIST.VAR1*LIST.VAR2*LIST.VAR3*LIST.VAR4',
		);
  foreach (array('1', '2', '3', '4') as $list_col)
    {
      $colname = 'LIST.VAR' . $list_col;
      if (!isset($form[$colname . '_MAX']))
	$form[$colname . '_MAX'] = '5';

      foreach (array('1', '2', '3', '4', '5') as $list_row)
	{
	  $rowname = $colname . '_' . $list_row;
	  if (!isset($form[$rowname]))
	    $form[$rowname] = '';
	}
    }

  /*
   * VAR7 and VAR 8 is a constraint of times during which
   * courses meet
   */
  $form['VAR7'] = '';
  $form['VAR8'] = '';

  /* ``course title keywords'' */
  $form['VAR3'] = '';

  /* ? */
  $form['VAR6'] = '';
  $form['VAR21'] = '';

  /* instructor's last name */
  $form['VAR9'] = '';

  /*
   * VAR10 through VAR16 are Monday through Sunday checkboxes
   * for days of the week that classes meet.
   *
   * But we specify no days of the week to avoid this being a
   * constraint ;-).
   */
  /*
    for ($day = 10; $day <= 16; $day ++)
    $form['VAR' . $day] = '';
  */

  /*
   * pages is populated by preg_match() below after the first looping.
   */
  $pages = array(1 => 0, 2=> 1);
  while ($pages[1] < $pages[2])
    {
      $html = calvin_crawl_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log, $form));

      $results_dom = new DOMDocument();
      $results_dom->loadHTML($html);	

      $list_done = FALSE;
      for ($list_row = 1; !$list_done; $list_row ++)
	{
	  /* either 'Open' (or 'Closed'?) */
	  $openness = dom_input_value($results_dom, 'LIST.VAR1_' . $list_row);
	  $sec_short_title = dom_id_content($results_dom, 'SEC_SHORT_TITLE_' . $list_row);
	  $sec_meetings_info = dom_id_content($results_dom, 'SEC_MEETING_INFO_' . $list_row);

	  /* check if we're done with this particular page */
	  if (!strlen($openness) && !strlen($sec_short_title) && !strlen($sec_meetings_info))
	    {
	      $list_done = TRUE;
	      break;
	    }

	  /*
	   * the same info below should be gettable with 
	   * dom_id_content($results_dom, 'SEC_FACULTY_INFO_' . $list_row);
	   */
	  $faculty_name = dom_input_value($results_dom, 'SEC.FACULTY.INFO_' . $list_row);
	  $credits = dom_input_value($results_dom, 'SEC.MIN.CRED_' . $list_row); /* or id="SEC_FACULTY_INFO_$list_row" */
	  $comment = dom_id_content($results_dom, 'SEC_COMMENTS_' . $list_row); /* or name="SEC.COMMENTS_$list_row" */
	  $short_title_onclick = $results_dom->getElementById('SEC_SHORT_TITLE_' . $list_row)->getAttribute('onclick');

	  /* parse */
	  $section_id = Section::parse($sec_short_title);
	  $synonym = NULL;
	  $title = NULL;
	  if (preg_match(';\(([0-9]+)\)(.*);', $sec_short_title, $matches))
	    {
	      $synonym = $matches[1];
	      $title = $matches[2];
	    }

	  school_crawl_logf($school_crawl_log, 10, "");
	  school_crawl_logf($school_crawl_log, 10, implode('-', $section_id) . ': ' . $sec_short_title);
	  school_crawl_logf($school_crawl_log, 10, $openness);
	  school_crawl_logf($school_crawl_log, 10, $sec_meetings_info);
	  school_crawl_logf($school_crawl_log, 10, $faculty_name);
	  school_crawl_logf($school_crawl_log, 10, $credits);
	  school_crawl_logf($school_crawl_log, 10, $comment);
	  school_crawl_logf($school_crawl_log, 10, "synonym: %s", $synonym);
	  school_crawl_logf($school_crawl_log, 10, "title: %s", $title);

	  /*
	   * The input format for this is, thankfully, pretty rigid
	   * :-D. Example input format:
	   *
	   * '01/31/2011-05/11/2011 Lecture Monday, Wednesday 01:00PM - 03:50PM, Spoelhof Center, Room 101'
	   *
	   * OR
	   *
	   * '01/31/2011-05/18/2011 Practicum Days to be Announced, Times to be AnnouncedTo Be Arranged, Room TBA'
	   *
	   * OR
	   *
	   * '01/31/2011-05/12/2011 Music Ensemble Monday, Wednesday, Thursday, Friday 03:30PM - 04:20PM, Covenant Fine Arts Center, Room 135'
	   *
	   * OR, per
	   * https://protofusion.org/bugzilla/show_bug.cgi?id=109 , we
	   * must parse the following on the main listing page and
	   * then parse more on the ``course details'' page:
	   *
	   * '09/06/2011-12/16/2011 Lecture Tuesday, Wednesday, Friday 12:30PM - 01:20PM, Science Building, Room 276 (more)...'
	   *
	   * The more on the ``course details'' page:
	   *
	   * '09/06/2011-12/16/2011 Lecture Tuesday, Wednesday, Friday 12:30PM - 01:20PM, Science Building, Room 276 09/06/2011-12/16/2011 Lecture Thursday 10:30AM - 12:20PM, Science Building, Room 276'
	   *
	   * Looks like in this last case parsing from right-to-left
	   * will be best.
	   *
	   * In the second case.... we'll just ignore the section. In
	   * the third case, we have to be careful about parsing out
	   * Monday.
	   *
	   * At this point, we don't parse most tokens. We group them
	   * off. We get the first date, the second date, the type
	   * ('Lecture', 'Practicum', or some other unknown value),
	   * the list of days of week the section meets, the start
	   * time, the end time, and then the meeting location.
	   */
	  if (strpos($sec_meetings_info, 'Times to be Announced') !== FALSE
	      || strpos($sec_meetings_info, 'Days to be Announced') !== FALSE)
	    {
	      school_crawl_logf($school_crawl_log, 8, 'Skipping class because of incomplete meeting time information: '
				. implode('-', $section_id) . ' has meeting info of `'
				. $sec_meetings_info . '\'');
	      $skipped_sections['incomplete meeting info'] ++;
	      /* Still add to have less confusing autocomplete */
	      calvin_crawl_course_add($semester, $section_id['department'], $section_id['course'], $title);
	      continue;
	    }

	  /*
	   * Check whether or not we have to pursue details on the
	   * ``course detail page''. If we do, we might as well just
	   * parse the line of information available there instead of
	   * the same from the main listing page.
	   */
	  if (preg_match('; \\(more\\)...$;', $sec_meetings_info)
	      && preg_match(';^javascript:window\\.open\\(\'(.*?[^\\\\])\',;', $short_title_onclick, $short_title_onclick_matches))
	    {
	      $more_details_url = $short_title_onclick_matches[1];
	      $more_details_uri = strstr($uri, '?', TRUE) . $more_details_url;

	      school_crawl_logf($school_crawl_log, 8, 'Fetching extra course information page for %s-%s-%s from %s.',
				$section_id['department'], $section_id['course'], $section_id['section'],
				$more_details_uri);
	      $more_details_html = calvin_crawl_geturi($more_details_uri, $cookies, $school_crawl_log);
	      $more_details_dom = new DOMDocument();
	      $more_details_dom->loadHTML($more_details_html);

	      /* Hopefully 'LIST_VAR12_1' is pretty constant... */
	      foreach ($more_details_dom->getElementById('LIST_VAR12_1')->childNodes as $more_details_child)
		{
		  if ($more_details_child->nodeType != XML_TEXT_NODE)
		    continue;
		  $sec_meetings_info = $more_details_child->wholeText;
		  break;
		}
	      school_crawl_logf($school_crawl_log, 9, "Result of fetching additional meeting information on next line(s):\n%s",
			       $sec_meetings_info);
	    }

	  /*
	   * If we have a course with multiple section_meetings, then
	   * $sec_meetings_info is split into each meeting by a
	   * "\n"
	   */

	  foreach (explode("\n", $sec_meetings_info) as $sec_meeting_info)
	    {
	      if (!preg_match(';^([0-9]{2}/[0-9]{2}/[0-9]{4})-([0-9]{2}/[0-9]{2}/[0-9]{4}) (([^ ,]+ )+)([^0-9]+) ([^ ]+) - ([^ ]+), (.*)$;', $sec_meeting_info, $meeting_info_matches))
	    {
	      school_crawl_logf($school_crawl_log, 8, 'Unable to parse calvin section meeting info string into start/end/days information for '
				. implode('-', $section_id) . ': ``' . $sec_meeting_info . '\'\'');
	      $skipped_sections['invalid meeting info format'] ++;
	      /*
	       * Still add at least the course to the semester so that
	       * it shows up in autocmoplete.
	       */
	      calvin_crawl_course_add($semester, $section_id['department'], $section_id['course'], $title);
	      continue;
	    }
	  $date_start = $meeting_info_matches[1];
	  $date_end = $meeting_info_matches[2];
	  /* e.g., 'Lecture', 'Practicum' */
	  $meeting_type = school_crawl_meeting_type($meeting_info_matches[3]);

	  $days = school_crawl_days_format($school_crawl_log, explode(', ', $meeting_info_matches[5]));
	  $time_start = school_crawl_time_format(strptime($meeting_info_matches[6], '%I:%M%p'));
	  $time_end = school_crawl_time_format(strptime($meeting_info_matches[7], '%I:%M%p'));
	  $meeting_place = $meeting_info_matches[8];

	  foreach (array('date_start', 'date_end', 'meeting_type', 'days', 'time_start', 'time_end', 'meeting_place', 'meeting_type') as $var)
	    school_crawl_logf($school_crawl_log, 10, "%s:%s", $var, ${$var});

	  $semester->section_meeting_add($section_id['department'], $section_id['course'], $title, $section_id['section'], $synonym,
					 new SectionMeeting($days, $time_start, $time_end, $meeting_place, $meeting_type, $faculty_name));

	  /*
	   * Try to update semester's longetivity stats to help the
	   * school_semester_guess() function:
	   */
	  $date_start_time = strptime($date_start, '%m/%d/%Y');
	  $date_end_time = strptime($date_end, '%m/%d/%Y');
	  if ($date_start_time !== FALSE)
	    {
	      $date_start_time = school_crawl_mktime($date_start_time);
	      if (!$semester_start_min || $semester_start_min > $date_start_time)
		$semester_start_min = $date_start_time;
	    }
	  if ($date_end_time !== FALSE)
	    {
	      $date_end_time = school_crawl_mktime($date_end_time);
	      if ($semester_end_max < $date_end_time)
		$semester_end_max = $date_end_time;
	    }
	}
	}

      if (!preg_match(';Page ([0-9]+) of ([0-9]+)\</td\>$;m', $html, $pages))
	{
	  school_crawl_logf($school_crawl_log, 0, 'Unable to determine the number of pages in this Calvin resultset');
	  break;
	}

      school_crawl_logf($school_crawl_log, 8, "calvin_crawl(): finished page %d of %d with %d courses.", $pages[1], $pages[2], $list_row - 1);

      $form = array(
		    'ACTION*Grp:WSS.COURSE.SECTIONS' => 'NEXT',
		    );
    }

  $has_stat = FALSE;
  foreach ($skipped_sections as $reason => $num)
    {
      if (!$num)
	continue;
      if (!$has_stat)
	school_crawl_logf($school_crawl_log, 7, 'Skipped some sections for <reason>: <number skipped>:');
      school_crawl_logf($school_crawl_log, 7, "%s: %d", $reason, $num);
    }

    $semester->time_end_set($semester_end_max);
    $semester->time_start_set($semester_start_min);

    /*
     * Calculate lab-based course dependencies.
     */
    school_crawl_logf($school_crawl_log, 7, 'Adding implicit lab dependencies.');
    foreach ($semester->departments_get() as $department)
      foreach ($semester->department_classes_get($department) as $course)
        {
	  $the_course = $semester->class_get($department, $course);
	  $lab_course = $semester->class_get($department, $course . 'L');
	  if (!empty($lab_course))
	    {
	      $the_course->dependency_add($lab_course);
	      school_crawl_logf($school_crawl_log, 8, "Adding dependency of %s-%s for %s-%s.",
				$department, $course . 'L', $department, $course);
	    }
	}

    $semesters[] = $semester;

    school_crawl_logf($school_crawl_log, 6, "");
    }

  return 0;
}

/**
 * \brief
 *   Find an <input /> element and return its value attribute.
 *
 * \param $domdocument
 *   The DOMDocument to search.
 * \param $name
 *   The name attribute of the <input /> element.
 * \return
 *   The value attribute of the input element or NULL if not found.
 */
function dom_input_value($domdocument, $name)
{
  $xpath = new DOMXPath($domdocument);
  $input_node_list = $xpath->query('/descendant::input[attribute::name="' . $name . '"]');

  if (!$input_node_list->length)
    return NULL;
  $input_node = $input_node_list->item(0);
  if (!$input_node->hasAttribute('value'))
    return NULL;
  return $input_node->getAttribute('value');
}

/**
 * \brief
 *   Returns the content of an element with the given ID.
 *
 * A convenience function.
 *
 * \param $domdocument
 *   A DOMDocument to search.
 * \param $id
 *   The id attribute of the element whose content are requested.
 * \return
 *   A UTF-8 string of the contents of the given element or NULL if
 *   the element isn't found.
 */
function dom_id_content($domdocument, $id)
{
  $node = $domdocument->getElementById($id);
  if ($node)
    {
      return $node->nodeValue;
    }
  return NULL;
}

/**
 * \brief
 *   Searches for and removes a <noscript/> element.
 *
 * The WebAdvisor likes to put <noscript/> in a docs <head />, which
 * is quite bad invalid HTML so that DOM can't handle it.
 *
 * \param $html
 *   The input HTML to filter.
 * \return
 *   The fixed HTML.
 */
function calvin_crawl_noscript_filter($html)
{
  return preg_replace(';\<(noscript)\>.*?\</\1\>;s', '', $html);
}

/**
 * \brief
 *   Follows a URL with support for WebAdvisor's silly TOKENIDX=
 *   thing.
 *
 * Automatically filters with calvin_crawl_noscript_filter().
 *
 * \param $uri
 *   The URL.
 * \param $cookies
 *   The cookies (yum!).
 * \param $school_crawl_log
 *   The school_crawl_log.
 */
function calvin_crawl_geturi(&$uri, array &$cookies, &$school_crawl_log)
{
  if (strpos($uri, 'TOKENIDX') === FALSE)
    {
      if (strpos($uri, '?') === FALSE)
	$uri .= '?';
      else
	$uri .= '&';

      /* Starting value. */
      $uri .= 'TOKENIDX=NULL';
    }

  $token_html = calvin_crawl_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log));

  if (!preg_match('/setWindowHTML\(\'\', \'([0-9]+)\'\);/', $token_html, $matches))
    return $token_html;
$token = $matches[1];

  school_crawl_logf($school_crawl_log, 7, "Using WebAdvisor token: %s.", $token);
  school_crawl_logf($school_crawl_log, 7, "");

  /*
   * setWindowHTML() will first remove the query string parameters
   * 'CLONE' and 'FORCEIDX'. Then it appends TOKENIDX=<token> to the
   * query parameters.
   *
   * Example, where TOKENIDX does not start out as NULL but where a
   * CLONE=Y command is being sent:
   *
   * Input: HTTPS://kvdata.calvin.edu/walive/WebAdvisor?TYPE=P&PID=ST-WESTS13C&CLONE=Y&CLONE_PROCESS=Y&SPAUKQ=708501792841963&CONSTITUENCY=WBST&TOKENIDX=1507971558
   *
   * Result: HTTPS://kvdata.calvin.edu/walive/WebAdvisor?TYPE=P&PID=ST-WESTS13C&CLONE_PROCESS=Y&SPAUKQ=708501792841963&CONSTITUENCY=WBST&TOKENIDX=2281086932
   */
  $uri = preg_replace('/([?&])TOKENIDX=[^&]+/', '$1TOKENIDX=' . $token,
		      preg_replace('/([?&])(CLONE|FORCEIDX)=[^&]+&?/', '$1', $uri));

  return calvin_crawl_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log));
}

/**
 * \brief
 *   Add a course to a semester if that semester doesn't yet have this
 *   course.
 *
 * \param $semester
 *   The semester to which the course should be appended.
 * \param $deparmtent
 *   The department of the course to add.
 * \param $course_id
 *   The course_id which, with the department string, forms a
 *   fully-qualified course_id.
 */
function calvin_crawl_course_add(Semester $semester, $department, $course_id, $title)
{
  if ($semester->class_get($department, $course_id) == NULL)
    $semester->class_add(new Course($department . '-' . $course_id, $title));
}
