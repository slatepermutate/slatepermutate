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
 * \param $semester
 *   The Semester object which I should populate.
 * \param $verbosity
 *   How verbose I should be. Sensicle range is from 0 through 10.
 */
function calvin_crawl(Semester $semester, $verbosity = 1)
{
  /**
   * collect a few pbasic stats
   */
  $skipped_sections = array('incomplete meeting info' => 0, 'invalid meeting info format' => 0);

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

  $baseuri = 'https://kvdata.calvin.edu/walive/WebAdvisor?CONSTITUENCY=WBST&type=P&pid=ST-WESTS12A&LASTTOKEN=NULL';

  $token_uri = $baseuri . '&TOKENIDX=NULL';
  $token_html = calvin_crawl_noscript_filter(school_crawl_geturi($token_uri, $cookies));
  if (!preg_match('/setWindowHTML\(\'\', \'([0-9]+)\'\);/', $token_html, $matches))
    {
      fprintf(STDERR, "Could not steal the token\n");
      return 1;
    }
  $token = $matches[1];

  if ($verbosity > 5)
    {
      echo 'token: ' . $token . "\n";
      echo "\n";
    }

  /*
   * here we have arrived at the main webadvisor screen which lists the
   * search form. From here, we can get a list of all of the departments
   * that Calvin College has and then know enough to query each
   * individual department for courses.
   */
  $uri = $baseuri . '&TOKENIDX=' . $token;
  $departments_html = calvin_crawl_noscript_filter(school_crawl_geturi($uri, $cookies));

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
	  || !$semester_node->hasAttribute('value'))
	continue;
      $semester_strs[$semester_node->getAttribute('value')] =
	$semester_node->nodeValue;
    }

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


  /* ARCT only has >=200 level courses */
  $dept = '';
  $course_level = '';
  $semester_str = substr($semester->year_get(), 2) . '/';
  switch ($semester->season_get())
    {
    case Semester::SEASON_SPRING:
      $semester_str .= 'SP';
      break;

    case Semester::SEASON_FALL:
      $semester_str .= 'FA';
      break;
    }
  if (!isset($semester_strs[$semester_str]))
    error_log('Couldn\'t find a semester in Calvin\'s database for ' . $semester_str . ' (' . $semester->season_get() . ', ' . $semester->year_get() . ')');


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
		);

  /*
   * other form items we're not querying but which need to be
   * sent blankly
   */
  $form += array(
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
      $html = calvin_crawl_noscript_filter(school_crawl_geturi($uri, $cookies, $form));

      $results_dom = new DOMDocument();
      $results_dom->loadHTML($html);	

      $list_done = FALSE;
      for ($list_row = 1; !$list_done; $list_row ++)
	{
	  /* either 'Open' (or 'Closed'?) */
	  $openness = dom_input_value($results_dom, 'LIST.VAR1_' . $list_row);
	  $sec_short_title = dom_id_content($results_dom, 'SEC_SHORT_TITLE_' . $list_row);
	  $sec_meeting_info = dom_id_content($results_dom, 'SEC_MEETING_INFO_' . $list_row);

	  /* check if we're done with this particular page */
	  if (!strlen($openness) && !strlen($sec_short_title) && !strlen($sec_meeting_info))
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

	  /* parse */
	  $section_id = Section::parse($sec_short_title);
	  $synonym = NULL;
	  if (preg_match(';\(([0-9]+)\);', $sec_short_title, $matches))
	    $synonym = $matches[1];

	  if ($verbosity > 6)
	    {
	      echo "\n";
	      echo implode('-', $section_id) . ': ' . $sec_short_title . "\n";
	      echo $openness . "\n";
	      echo $sec_meeting_info . "\n";
	      echo $faculty_name . "\n";
	      echo $credits . "\n";
	      echo $comment . "\n";
	    }

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
	   * In the second case.... we'll just ignore the section. In
	   * the last case, we have to be careful about parsing out
	   * Monday.
	   *
	   * At this point, we don't parse most tokens. We group them
	   * off. We get the first date, the second date, the type
	   * ('Lecture', 'Practicum', or some other unknown value),
	   * the list of days of week the section meets, the start
	   * time, the end time, and then the meeting location.
	   */
	  if (strpos($sec_meeting_info, 'Times to be Announced') !== FALSE
	      || strpos($sec_meeting_info, 'Days to be Announced') !== FALSE)
	    {
	      if ($verbosity > 2)
		error_log('Skipping class because of incomplete meeting time information: '
			  . implode('-', $section_id) . ' has meeting info of `'
			  . $sec_meeting_info . '\'');
	      $skipped_sections['incomplete meeting info'] ++;
	      continue;
	    }

	  if (!preg_match(';^([0-9]{2}/[0-9]{2}/[0-9]{4})-([0-9]{2}/[0-9]{2}/[0-9]{4}) (([^ ,]+ )+)([^0-9]+) ([^ ]+) - ([^ ]+), (.*)$;', $sec_meeting_info, $meeting_info_matches))
	    {
	      error_log('Unable to parse calvin section meeting info string into start/end/days information for '
			. implode('-', $section_id) . ': ``' . $sec_meeting_info . '\'\'');
	      $skipped_sections['invalid meeting info format'] ++;
	      continue;
	    }
	  $date_start = $meeting_info_matches[1];
	  $date_end = $meeting_info_matches[2];
	  /* e.g., 'Lecture', 'Practicum' */
	  $meeting_type = $meeting_info_matches[3];
	  $days = school_crawl_days_format(explode(', ', $meeting_info_matches[5]));
	  $time_start = school_crawl_time_format(strptime($meeting_info_matches[6], '%I:%M%p'));
	  $time_end = school_crawl_time_format(strptime($meeting_info_matches[7], '%I:%M%p'));
	  $meeting_place = $meeting_info_matches[8];

	  if ($verbosity > 5)
	    foreach (array('date_start', 'date_end', 'meeting_type', 'days', 'time_start', 'time_end', 'meeting_place') as $var)
	      echo $var . ':' . ${$var} . "\n";

	  $section = new Section($section_id['section'], array(new SectionMeeting($days, $time_start, $time_end, $meeting_place)), $synonym, $faculty_name);
	  $semester->section_add($section_id['department'], $section_id['course'], $section);
	}

      if (!preg_match(';Page ([0-9]+) of ([0-9]+)\</td\>$;m', $html, $pages))
	{
	  error_log('Unable to determine the number of pages in this Calvin resultset');
	  break;
	}

      if ($verbosity > 0)
	{
	  echo 'calvin_crawl(): finished page ' . $pages[1] . ' of ' . $pages[2] . ' with ' . ($list_row - 1) . " courses.\n";
	}

      $form = array(
		    'ACTION*Grp:WSS.COURSE.SECTIONS' => 'NEXT',
		    );
    }

  $has_stat = FALSE;
  if ($verbosity > 1)
    foreach ($skipped_sections as $reason => $num)
      {
	if (!$num)
	  continue;
	if (!$has_stat)
	  error_log('Skipped some sections for <reason>: <number skipped>:');
	error_log($reason . ': ' . $num);
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
