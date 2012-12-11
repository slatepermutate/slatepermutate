<?php /* -*- mode: php; -*- */
/*
 * Copyright 2012 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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
 * \file
 *
 * A crawler for the WebAdvisor webapp.
 */

$incdir = dirname(__FILE__) . DIRECTORY_SEPARATOR;
require_once $incdir . 'class.semester.inc';
require_once $incdir . 'class.course.inc';
require_once $incdir . 'class.section.php';
require_once $incdir . 'class.section_meeting.inc';

define('_SCHOOL_CRAWL_WEBADVISOR_START_FORM', '?type=P&pid=ST-WESTS12A&LASTTOKEN=NULL');

function _school_crawl_webadvisor_common_prep(array &$school, array &$options)
{
  $school += array('webadvisor_url' => $school['url'] . 'WebAdvisor');
   $options += array(
    'season_mapper' => 'school_crawl_webadvisor_season_mapper',
    'curlsetup_hook' => NULL,
  );
}

/*
 * \brief
 *   Crawl the list of semesters available from a
 *   WebAdvisor-compatible school.
 *
 * \param $school
 *   The school’s info array/handle.
 * \param $semesters
 *   The array to populate with various semesters available at this
 *   college.
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 * \param $season_mapper
 *   A function($term, $term_value) which maps term names onto
 *   semester/season names. See
 *   school_crawl_webadvisor_season_mapper() for the default
 *   implementation.
 */
function school_crawl_webadvisor_semester_list(array $school, array &$semesters, &$school_crawl_log, array $options = array())
{
  _school_crawl_webadvisor_common_prep($school, $options);

  $cookies = array();
  $uri = $school['webadvisor_url'] . _SCHOOL_CRAWL_WEBADVISOR_START_FORM;
  $semesters_html = school_crawl_webadvisor_geturi($uri, $cookies, $school_crawl_log, $options);

  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML($semesters_html);

  /*
   * Discover the available semesters
   */
  $semesters_var1 = $semesters_dom->getElementById('VAR1');
  if (empty($semesters_var1))
    {
      school_crawl_logf($school_crawl_log, 0, "Error: Unable to load list of semesters.");
      return 1;
    }
  $semesters_select_nodes = $semesters_var1->childNodes;
  foreach ($semesters_select_nodes as $semester_node)
    {
      if ($semester_node->tagName != 'option'
	  || !$semester_node->hasAttribute('value')
	  || !strlen($semester_node->getAttribute('value')))
	continue;

      $term = $semester_node->textContent;
      $term_value = $semester_node->getAttribute('value');
      $semester = $options['season_mapper']($term, $term_value, $school_crawl_log);

      /*
       * We need a way to map a semester back to a list of
       * term_values. We can tack an extra member variable onto any
       * object in PHP, so we use that method.
       */
      if (!empty($semester))
	if (empty($semesters[$semester->id()]))
	  {
	    $semester->_school_crawl_webadvisor_term_values = array($term_value);
	    $semesters[$semester->id()] = $semester;
	  }
	else
	  /*
	   * A semester associated with this year/season already
	   * exists. Append an additional term value to be associated
	   * with this Semester so that they can be aggregated when
	   * crawled later.
	   */
	  $semesters[$semester->id()]->_school_crawl_webadvisor_term_values[] = $term_value;
    }

  return 0;
}

function school_crawl_webadvisor_geturi(&$uri, array &$cookies, &$school_crawl_log, array $options)
{
  /**
   * We have to handle the case where the user is first browing to
   * WebAdvisor. For example, with the ST-WESTS12A sequence:
   *
   * Start the ST-WESTS12A sequence.
   *
   * 1. WebAdvisor?type=P&pid=ST-WESTS12A&LASTTOKEN=NULL
   *    <body onload="javascript:getWindowHTML();">
   *
   *    Calls javascript:getWindowHTML(). This merely adds
   *    TOKENIDX=NULL to the query string, so we can skip this step
   *    and just have TOKENIDX=NULL.
   *
   * 2. WebAdvisor?type=P&pid=ST-WESTS12A&LASTTOKEN=NULL&TOKENIDX=NULL
   *    <body onload="javascript:setWindowHTML(”, '7699844013');">
   *
   * 3. WebAdvisor?type=P&pid=ST-WESTS12A&TOKENIDX=7699844013 In #2,
   *    the second argument to setWindowHTML() is random. Thus, we
   *    have to capture this value and set it as GET parameter named
   *    “TOKENIDX”.
   */
  if (strpos($uri, 'TOKENIDX') === FALSE)
    {
      if (strpos($uri, '?') === FALSE)
	$uri .= '?';
      else
	$uri .= '&';

      /* Starting value. */
      $uri .= 'TOKENIDX=NULL';
    }

  $html = school_crawl_webadvisor_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log, NULL, FALSE, $options['curlsetup_hook']));

  if (!preg_match('/setWindowHTML\\(\'\', \'([0-9]+)\'\\);/', $html, $matches))
    /*
     * The user already had a valid TOKENIDX, so we’re good to go.
     */
    return $html;

  $token = $matches[1];
  school_crawl_logf($school_crawl_log, 7, "Using WebAdvisor TOKENIDX=%s.", $token);

  /*
   * setWindowHTML() will first remove the query string parameters
   * 'CLONE' and 'FORCEIDX'. Then it appends TOKENIDX=<token> to the
   * query parameters.
   *
   * Example, where TOKENIDX does not start out as NULL but where a
   * CLONE=Y command is being sent:
   *
   * Input: HTTPS://kvdata.calvin.edu/walive/WebAdvisor?TYPE=P&PID=ST-WESTS13C&CLONE=Y&CLONE_PROCESS=Y&SPAUKQ=708501792841963&TOKENIDX=1507971558
   *
   * Result: HTTPS://kvdata.calvin.edu/walive/WebAdvisor?TYPE=P&PID=ST-WESTS13C&CLONE_PROCESS=Y&SPAUKQ=708501792841963&TOKENIDX=2281086932
   */
  $uri = preg_replace('/([?&])TOKENIDX=[^&]+/', '$1TOKENIDX=' . $token,
		      preg_replace('/([?&])(CLONE|FORCEIDX)=[^&]+&?/', '$1', $uri));

  return school_crawl_webadvisor_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log, NULL, FALSE, $options['curlsetup_hook']));
}

/**
 * \brief
 *   Searches for and removes a <noscript/> element.
 *
 * The WebAdvisor likes to put <noscript/> in a docs <head /> and
 * place things other than <link/>, <meta/>, and <style/> in this
 * <noscript/>. This is invalid HTML
 * (http://www.w3.org/TR/html5/the-noscript-element.html#the-noscript-element)
 * and not handled by libxml2’s DOM.
 *
 * \param $html
 *   The input HTML to filter.
 * \return
 *   The fixed HTML.
 */
function school_crawl_webadvisor_noscript_filter($html)
{
  return preg_replace(';\<(noscript)\>.*?\</\1\>;s', '', $html);
}

/**
 * \brief
 *   Map a term name onto a season and generate the appropriate
 *   Semester object.
 *
 * \param $term
 *   The human-friendly term.
 * \param $term_value
 *   The form value of this term.
 * \return
 *   NULL if unable to generate the Semester, otherwise an empty
 *   Semester object with its year and season set.
 */
function school_crawl_webadvisor_season_mapper($term, $term_value, &$school_crawl_log)
{
  if (!preg_match('/(^|[^\d])(\d{4})($|[^\d])/', $term, $matches))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to interpret “%s” with form value of “%s” as specifying a particular year.",
			$term, $term_value);
      return NULL;
    }
  $year = $matches[2];
  $term_minusyear = trim(str_replace($year, '', $term));

  list($season) = explode(' ', strtolower($term_minusyear));
  if (empty($season))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to interpret “%s” with form value of “%s” as specifying a particular season.",
			$term, $term_value);
      return NULL;
    }

  /*
   * Try to coerce into a valid season name. For example, will coerce
   * 'sp' into 'spring'.
   */
  $season_strlen = strlen($season);
  foreach (Semester::seasons_get_all() as $valid_season)
    if (!strncmp($valid_season, $season, min(strlen($valid_season), $season_strlen)))
      $season = $valid_season;

  school_crawl_logf($school_crawl_log, 9, "Interpreting “%s” term as %s-%s.", $term, $year, $season);
  return new Semester($year, $season);
}

/**
 * \brief
 *   Map a particular Semester onto a list of term_values for using as
 *   “VAR1” in the initial schedule form.
 *
 * \param $school
 *   The school handle.
 * \param $semester
 *   The Semester to map onto a list of term_values.
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 * \param $season_mapper
 *   The season_mapper used with
 *   school_crawl_webadvisor_semester_list().
 * \return
 *   An array, possibly empty, of term_values associated with this
 *   Semester or NULL on error.
 */
function _school_crawl_webadvisor_semester_toterms(array $school, Semester $semester, &$school_crawl_log, array $options)
{
  if (!empty($semester->_school_crawl_webadvisor_term_values))
    return $semester->_school_crawl_webadvisor_term_values;

  $semesters = array();
  $ret = school_crawl_webadvisor_semester_list($school, $semesters, $school_crawl_log, $options);
  if ($ret)
    {
      school_crawl_logf($school_crawl_log, 4, "Unable to map Semester “%s” onto a term_value because crawling the semester list failed.", $semester);
      return NULL;
    }
  if (empty($semesters[$semester->id()]))
    {
      school_crawl_logf($school_crawl_log, 4, "Unable to map Semester “%s” onto a semester because no matching semester was found in the semester list.", $semester);
      return NULL;
    }
  return $semesters[$semester->id()]->_school_crawl_webadvisor_term_values;
}

/**
 * \brief
 *   Crawl the courses for a semester from a WebAdvisor instance.
 *
 * There may be multiple terms associated with a particular
 * semester. For example,
 * https://solomon.cornerstone.edu/COLLIVE/WebAdvisor?CONSTITUENCY=WBST&type=P&pid=ST-WESTS12A
 * lists overlapping terms such as “Spring 2013 Undergraduate”,
 * “Spring 2013 Seminary”, and “SP 2013 GRD (MAML/MABS)”.
 *
 * \param $school
 *   The school handle.
 * \param $semester
 *   The Semester to crawl.
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 * \param $season_mapper
 *   The season_mapper used with
 *   school_crawl_webadvisor_semester_list() if any.
 */
function school_crawl_webadvisor_semester(array $school, Semester $semester, &$school_crawl_log, array $options = array())
{
  _school_crawl_webadvisor_common_prep($school, $options);

  $cookies = array();
  $uri = $school['webadvisor_url'] . _SCHOOL_CRAWL_WEBADVISOR_START_FORM;
  $html = school_crawl_webadvisor_geturi($uri, $cookies, $school_crawl_log, $options);
  $form_uri = $uri;
  $seed_dom = new DOMDocument();
  $seed_dom->loadHTML($html);
  $seed_form = _school_crawl_webadvisor_form($seed_dom, $school_crawl_log);
  if (empty($seed_form))
    return 1;
  $return_url = reset($seed_form['RETURN.URL']);

  /*
   * First, read all of the friendly subject/department names. They're
   * not in the output, but they're in the “Subjects” dropdown of
   * the input form. The <select name="LIST.VAR1_1" id="LIST_VAR1_1"/>
   * is associated with subjects/departments.
   */
  $department_var1_list = array();
  foreach (school_crawl_form_select_array($seed_dom->getElementById('LIST_VAR1_1')) as $department_id => $department_name)
    {
      $semester->department_name_set($department_id, trim(reset($department_name)));
      $department_var1_list[] = $department_id;
    }

  foreach (_school_crawl_webadvisor_semester_toterms($school, $semester, $school_crawl_log, $options) as $semester_str)
  while (count($department_var1_list))
    {
      /* Start back on the form page... */
      $uri = $form_uri;

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
  school_crawl_logf($school_crawl_log, 6, 'Using %s for a semester form value.',
		    $semester_str);
  $form = array('VAR1' => $semester_str,
		'LIST.VAR1_1' => '',
		'LIST.VAR2_1' => '',

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
   * Fill in some semesters.
   */
  foreach (array('1', '2', '3', '4', '5') as $var1_row)
    if (count($department_var1_list))
      {
	$form['LIST.VAR1_' . $var1_row] = array_shift($department_var1_list);
      }

  /*
   * VAR7 and VAR 8 is a constraint of times during which
   * courses meet
   */
  $form['VAR7'] = '';
  $form['VAR8'] = '';

  /* “course title keywords” */
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

  $skipped_sections = array('incomplete meeting info' => 0, 'invalid meeting info format' => 0);
  /*
   * pages is populated by preg_match() below after the first looping.
   */
  $pages = array(1 => 0, 2 => 1);
  while ($pages[1] < $pages[2])
    {
      $html = school_crawl_webadvisor_noscript_filter(school_crawl_geturi($uri, $cookies, $school_crawl_log, $form, FALSE, $options['curlsetup_hook']));

      $results_dom = new DOMDocument();
      $results_dom->loadHTML($html);
      $results_form = _school_crawl_webadvisor_form($results_dom, $school_crawl_log);
      if (empty($results_form))
	return 1;

      $list_done = FALSE;
      for ($list_row = 1; !$list_done; $list_row ++)
	{
	  /* either 'Open' (or 'Closed'?) */
	  $openness = empty($results_form['LIST.VAR1_' . $list_row]) ? NULL : reset($results_form['LIST.VAR1_' . $list_row]);
	  $sec_short_title = _school_crawl_webadvisor_dom_id_content($results_dom, 'SEC_SHORT_TITLE_' . $list_row);
	  $sec_meetings_info = _school_crawl_webadvisor_dom_id_content($results_dom, 'SEC_MEETING_INFO_' . $list_row);

	  /* check if we're done with this particular page */
	  if (!strlen($openness) && !strlen($sec_short_title) && !strlen($sec_meetings_info))
	    {
	      $list_done = TRUE;
	      break;
	    }

	  /*
	   * The same info below should be retrievable with 
	   * _school_crawl_webadvisor_dom_id_content($results_dom, 'SEC_FACULTY_INFO_' . $list_row);
	   */
	  $faculty_name = reset($results_form['SEC.FACULTY.INFO_' . $list_row]);
	  $credits = reset($results_form['SEC.MIN.CRED_' . $list_row]); /* or id="SEC_FACULTY_INFO_$list_row" */
	  $comment = _school_crawl_webadvisor_dom_id_content($results_dom, 'SEC_COMMENTS_' . $list_row); /* or name="SEC.COMMENTS_$list_row" */
	  $short_title_onclick = $results_dom->getElementById('SEC_SHORT_TITLE_' . $list_row)->getAttribute('onclick');

	  /* parse */
	  $section_id = Section::parse($sec_short_title);
	  $synonym = NULL;
	  $title = NULL;
	  if (preg_match(';\(([0-9]+)\)(.*);', $sec_short_title, $matches))
	    {
	      $synonym = $matches[1];
	      $title = trim($matches[2]);
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
	   * then parse more on the “course details” page:
	   *
	   * '09/06/2011-12/16/2011 Lecture Tuesday, Wednesday, Friday 12:30PM - 01:20PM, Science Building, Room 276 (more)...'
	   *
	   * The more on the “course details” page:
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
	      school_crawl_webadvisor_course_add($semester, $section_id['department'], $section_id['course'], $title);
	      continue;
	    }

	  /*
	   * Check whether or not we have to pursue details on the
	   * “course detail page”. If we do, we might as well just
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
	      $more_details_html = school_crawl_webadvisor_geturi($more_details_uri, $cookies, $school_crawl_log, $options);
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
	      school_crawl_logf($school_crawl_log, 8, 'Unable to parse section meeting info string into start/end/days information for '
				. implode('-', $section_id) . ': “' . $sec_meeting_info . '”');
	      $skipped_sections['invalid meeting info format'] ++;
	      /*
	       * Still add at least the course to the semester so that
	       * it shows up in autocmoplete.
	       */
	      school_crawl_webadvisor_course_add($semester, $section_id['department'], $section_id['course'], $title);
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

	  $date_start_time = strptime($date_start, '%m/%d/%Y');
	  $date_end_time = strptime($date_end, '%m/%d/%Y');
	  if ($date_start_time !== FALSE)
	    $date_start_time = school_crawl_gmmktime($date_start_time, -5 * 60*60);
	  else
	    $date_start_time = NULL;
	  if ($date_end_time !== FALSE)
	    $date_end_time = school_crawl_gmmktime($date_end_time, -5 * 60*60) + 24*60*60;
	  else
	    $date_end_time = NULL;

	  $semester->section_meeting_add($section_id['department'], $section_id['course'], $title, $section_id['section'], $synonym,
					 new SectionMeeting($days, $time_start, $time_end, $meeting_place, $meeting_type, $faculty_name, $date_start_time, $date_end_time), 'default', $credits);

	    }
	}

      if (!preg_match(';Page ([0-9]+) of ([0-9]+)\</td\>$;m', $html, $pages))
	{
	  school_crawl_logf($school_crawl_log, 0, 'Unable to determine the number of pages in this resultset');
	  break;
	}

      school_crawl_logf($school_crawl_log, 8, "%s(): finished page %d of %d with %d courses.", __FUNCTION__, $pages[1], $pages[2], $list_row - 1);

      $form = array(
		    'ACTION*Grp:WSS.COURSE.SECTIONS' => 'NEXT',
		    );
    }
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

  return 0;
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
function school_crawl_webadvisor_course_add(Semester $semester, $department, $course_id, $title)
{
  if ($semester->class_get($department, $course_id) == NULL)
    $semester->class_add(new Course($department . '-' . $course_id, $title));
}

/**
 * \brief
 *   Find the datatelform and run it through school_crawl_form().
 *
 * \return
 *   See school_crawl_form(), NULL if form not found.
 */
function _school_crawl_webadvisor_form($dom, array &$school_crawl_log)
{
  $xpath = new DOMXPath($dom);
  foreach ($xpath->query('.//form[@name="datatelform"]') as $dom_form)
    return school_crawl_form($dom_form);
  school_crawl_logf($school_crawl_log, 2, "Unable to find form[@name=\"datatelform\"].");
  return NULL;
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
function _school_crawl_webadvisor_dom_id_content($domdocument, $id)
{
  $node = $domdocument->getElementById($id);
  if ($node)
    {
      return $node->nodeValue;
    }
  return NULL;
}
