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
 * \brief
 *   Start a Hope crawling session.
 */
function _hope_crawl_start(array $school, &$uri, array &$cookies, &$dom, &$xpath, &$school_crawl_log)
{
  $cookies = array();
  $uri = 'http://plus.hope.edu/PROD/hxskschd.P_hopeSchedule';
  $dom = new DOMDocument();

  $html = school_crawl_geturi($uri, $cookies, $school_crawl_log);
  if (empty($html)
      || !$dom->loadHTML($html))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to load the HTML document necessary to enumerate %s's list of semesters.",
			$school['id']);
      return 1;
    }

  $xpath = new DOMXPath($dom);

  return 0;
}

/**
 * \brief
 *   Crawl the list of available semesters from Hope.
 *
 * Crawling starts at
 * http://plus.hope.edu/PROD/hxskschd.P_hopeSchedule . This is linked
 * to from http://hope.edu/registrar/nav/schedules.html and from
 * http://plus.hope.edu/ (which redirects to a PROD page which has
 * `Release 8.4.2'. The HTTP server claims to be ``Server:
 * Oracle-Application-Server-10g/10.1.2.0.2 Oracle-HTTP-Server''.
 *
 * \param $school
 *   The school handle for Hope College.
 * \param $semesters
 *   The array to which Semester objects shall be appended.
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 */
function hope_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  $uri = NULL;
  $cookies = array();
  $dom = NULL;
  $xpath = NULL;

  if ($ret = _hope_crawl_start($school, $uri, $cookies, $dom, $xpath, $school_crawl_log))
    return $ret;

  if (($dom_select_terms = $xpath->query('.//select[@name="term"]/option[string-length(@value) > 0]')) === FALSE
      || !$dom_select_terms->length)
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to find the <select name=\"term\"/> for %s.",
			$school['id']);
      return 1;
    }
  foreach ($dom_select_terms as $dom_select_term)
    {
      list($season, $year) = explode(' ', strtolower(trim($dom_select_term->textContent)));
      $semesters[] = new Semester($year, $season);
    }

  return 0;
}

function _hope_crawl_days_filter($day)
{
  return !empty($day) && strlen(trim($day));
}

function hope_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  $uri = NULL;
  $cookies = array();
  $dom = NULL;
  $xpath = NULL;

  if ($ret = _hope_crawl_start($school, $uri, $cookies, $dom, $xpath, $school_crawl_log))
    return $ret;

  if (($dom_select_terms = $xpath->query('.//select[@name="term"]/option[string-length(@value) > 0]')) === FALSE
      || !$dom_select_terms->length)
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to find the <select name=\"term\"/> for %s.",
			$school['id']);
      return 1;
    }

  foreach ($dom_select_terms as $dom_select_term)
    {
      list($season, $year) = explode(' ', strtolower(trim($dom_select_term->textContent)));
      if (!strcmp($year, $semester->year_get())
	  && !strcmp($season, $semester->season_get())
	  && $dom_select_term->hasAttribute('value'))
	break;
      unset($dom_select_term);
    }
  if (empty($dom_select_term))
    {
      school_crawl_logf($school_crawl_log, 4, "Unable to find the form input value associated with the %s semester.",
			$semester);
      return 1;
    }

  $semester_form_node = school_crawl_element_ancestor($dom_select_term, 'form');
  $semester_form = school_crawl_form($semester_form_node);
  $semester_form_action = $semester_form_node->getAttribute('action');
  $semester_form['term'] = $dom_select_term->getAttribute('value');

  foreach ($xpath->query('.//select[@name="sel_subj"]') as $dom_select_term)
    break;
  if (empty($dom_select_term))
    {
      school_crawl_logf($school_crawl_log, 4, "Unable to find Subject-selecting form input");
      return 1;
    }

  /*
   * Manually select all of the different sorts of subject materials
   * since selecting no subjects doesn't result in listing them all.
   */
  $semester_form['sel_subj'] = array();
  foreach (school_crawl_form_select_array($dom_select_term, FALSE) as $subject_name => $junk)
    $semester_form['sel_subj'][] = $subject_name;

  if (!empty($semester_form_action))
    $uri = school_crawl_url($uri, $semester_form_action);
  $sections_html = school_crawl_geturi($uri, $cookies, $school_crawl_log, $semester_form);

  /*
   * Get an HTML-based results page. We only get this page because it
   * has a <form /> which we can submit to get CSV.
   */
  $sections_dom = new DOMDocument();
  if (empty($sections_html)
      || !$sections_dom->loadHTML($sections_html))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to load section listings page.");
      return 1;
    }
  $sections_xpath = new DOMXPath($sections_dom);

  /* Look for the "Export to Excel" submit button */
  $sections_form = $sections_xpath->query('.//form[.//input[@type = "submit" and contains(@value, "xport")]]')->item(0);
  if (empty($sections_form))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to find CSV link for schedule.");
      return 1;
    }

  /* Get the CSV */
  $sections_form_action = $sections_form->getAttribute('action');
  if (!empty($sections_form_action))
    $uri = school_crawl_url($uri, $sections_form_action);
  $sections_csv = school_crawl_geturi($uri, $cookies, $school_crawl_log, school_crawl_form($sections_form));

  /*
   * Oracle likes to put random `"' into the middle of a quoted string
   * instead of properly escaping it like ``"This is a string with a
   * "" in it"''. This regex blasts away such doublequotes which are
   * not adjacent to delimiters (hopefully).
   */
  $sections_csv = preg_replace('/([^,\\n\\r])"([^,\\n\\r])/', '$1""$2', $sections_csv);
  $sections_csv = school_crawl_csv_parse($sections_csv, array('eof' => TRUE));
  /* Skip the introductory lines, seeking for the field headers */
  for ($i = 0; $i < count($sections_csv) && count($sections_csv[$i]) < 2; $i ++)
    ;

  $fields = array(
    'Status' => FALSE /*< OPEN, RESTRICTED, IN PROGRESS, or empty */,
    'Title' => FALSE /*< course name */,
    'Subject' => FALSE /*< subject id */,
    'Course Number' => FALSE,
    'Section Number' => FALSE,
    'CRN' => FALSE /*< section synonym */,
    'Cred' => FALSE /*< Number of credits, can be a range which would be formatted like "  1-4" */,
    /*
     * ex. "FA1", "FA2", "CH2" (online course?), "CD4", "SRS"
     * (seniors). If a course has multiple attributes, it will have
     * multiple lines following it with the attributes but no other
     * fields filled?
     */
    'Attr' => FALSE,
    /*
     * The first of 8 columns being Day + times. "M" (or "TBA"), "T",
     * "W", "R", "F", <saturday>?, <sunday>?, "1600-1800" or "TBA".
     */
    'Meeting Days/Times' => FALSE,
    'Location' => FALSE /*< The room or TBA */,
    'Capacity' => FALSE /*< Probably the maximum number of students */,
    'Actual' => FALSE /*< Possibly the current number of students? */,
    'Remainder' => FALSE  /*< Number of spots to be filled... */,
    'Instructor' => FALSE /*< The prof/instructor */,
    /*
     * The start/end dates in form of 07/02-07/27. This would be
     * particularly important for supporting half-semester
     * courses. Bug #122.
     */
    'Date' => FALSE,
    'Weeks' => FALSE /*< The total number of weeks the course meets */,
  );

  foreach ($sections_csv[$i] as $column => $name)
    if (!empty($name))
      $fields[$name] = $column;
  $expected_columns = max($fields);
  foreach ($fields as $name => $location)
    if ($location === FALSE)
      {
	school_crawl_logf($school_crawl_log, 2, "Cannot find column named %s in CSV. The column headings line looks like ``%s''.",
			  $name, implode(',', $sections_csv[$i]));
	return 1;
      }

  /* Label the days of the week and Times column */
  foreach (array('M', 'T', 'W', 'R', 'F', 'S', 'U', 'Times') as $offset => $name)
    $fields[$name] = $fields['Meeting Days/Times'] + $offset;

  for ($i ++; $i < count($sections_csv); $i ++)
    {
      $section_csv = $sections_csv[$i];

      if (count($section_csv) < $expected_columns)
	{
	  school_crawl_logf($school_crawl_log, 8, "Skipping row which has fewer entries than expected (%d): %s",
			    $expected_columns, implode(', ', $section_csv));
	  continue;
	}

      /*
       * If a section has multiple meetings, each extra meeting is
       * placed on a row following the first section's entry. However,
       * the course/synonym/section/subject are all blank on that
       * line. Therefore, we must propagate these values.
       */
      foreach (array(
	'subject_id' => 'Subject',
	'course_id' => 'Course Number',
	'title' => 'Title',
	'section_id' => 'Section Number',
	'synonym' => 'CRN',
	'instructor' => 'Instructor',
	'location' => 'Location',
      ) as $var => $field)
	if (strlen(trim($section_csv[$fields[$field]])))
	  ${$var} = trim($section_csv[$fields[$field]]);

      if ($section_csv[$fields['M']] == 'TBA'
	  || $section_csv[$fields['Times']] == 'TBA')
	{
	  $semester->class_add(new Course($subject_id . '-' . $course_id,
					  $section_csv[$fields['Title']]));
	  school_crawl_logf($school_crawl_log, 8, "Course %s-%s-%s has a section meeting with a TBA time, adding dummy course.",
			    $subject_id, $course_id, $section_id);
	  continue;
	}

      if (preg_match(',(\\d\\d)/(\\d\\d)-(\\d\\d)/(\\d\\d),', $section_csv[$fields['Date']], $matches))
	{
	  list(, $m_start, $d_start, $m_end, $d_end) = $matches;
	  if ($m_start && $d_start && $m_end && $d_end)
	    {
	      $y_start = $y_end = $semester->year_get();
	      if ($m_end < $m_start)
		$y_end ++;
	      $semester->time_start_set_test(gmmktime(0, 0, 0, $m_start, $d_start, $y_start));
	      $semester->time_end_set_test(gmmktime(0, 0, 0, $m_end, $d_end, $y_end));
	    }
	}

      if (trim($section_csv[$fields['U']]))
	school_crawl_logf($school_crawl_log, 0, "Section %d has sunday.", $synonym);
      $days = school_crawl_days_format($school_crawl_log, array_filter(array_slice($section_csv, $fields['M'], 7), '_hope_crawl_days_filter'));
      list($time_start, $time_end) = explode('-', $section_csv[$fields['Times']]);
      if (strlen($time_start) != 4 || strlen($time_end) != 4)
	{
	  school_crawl_logf($school_crawl_log, 4, "Section meeting (synonym=%s) has invalidly-formatted start time (%s) or end time (%s). Skipping.",
			    $synonym, $time_start, $time_end);
	  continue;
	}

      /*
       * Guessing the type of section_meeting: `attribute' of NSL
       * seems to be associated with labs. Matches `lab', `lab.', `
       * lab', ` labo'..., etc.
       */
      $type = 'lecture';
      if (preg_match('/(^|[^a-z])lab($|o|[^a-z])/i', $title))
	$type = 'lab';

      $section_meeting = new SectionMeeting($days, $time_start, $time_end,
					    $location,
					    $type,
					    $instructor);
      $semester->section_meeting_add($subject_id,
				     $course_id,
				     $title,
				     $section_id,
				     $synonym,
				     $section_meeting,
				     $type,
				     $section_csv[$fields['Cred']]);
    }
  return 0;
}