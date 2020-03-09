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
  $uri = 'https://schedule.hope.edu/';
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

function _hope_crawl_parse_term($term_text)
{
  if (preg_match(',(.*) (.*),', strtolower(trim($term_text)), $dom_select_terms_matches))
    {
      list(, $season, $year) = $dom_select_terms_matches;
      return array($year, str_replace(' ', '_', $season));
    }
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
      || !$dom_select_terms->length) {
    school_crawl_logf($school_crawl_log, 2, "Unable to find the <select name=\"term\"/> for %s.", $school['id']);
    return 1;
  }
  foreach ($dom_select_terms as $dom_select_term) {
    /* Support formats like “Spring 2013” and “May Term 2014”. */
    $term_text = $dom_select_term->textContent;
    list($year, $season) = _hope_crawl_parse_term($term_text);
    if ($year) {
      $semesters[] = new Semester($year, $season);
    } else {
      school_crawl_logf($school_crawl_log, 2, 'Unparsable semester name: “%s”.', $term_text);
    }
  }

  return 0;
}

function _hope_crawl_days_filter($day) {
  return !empty($day) && strlen(trim($day));
}

function hope_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  $uri = NULL;
  $cookies = array();
  $dom = NULL;
  $xpath = NULL;

  if ($ret = _hope_crawl_start($school, $uri, $cookies, $dom, $xpath, $school_crawl_log)) {
    return $ret;
  }

  if (($dom_select_terms = $xpath->query('.//select[@name="term"]/option[string-length(@value) > 0]')) === FALSE
      || !$dom_select_terms->length) {
    school_crawl_logf($school_crawl_log, 2, "Unable to find the <select name=\"term\"/> for %s.", $school['id']);
    return 1;
  }

  foreach ($dom_select_terms as $dom_select_term) {
    list($year, $season) = _hope_crawl_parse_term($dom_select_term->textContent);
    if (!strcmp($year, $semester->year_get())
        && !strcmp($season, $semester->season_get()) 
        && $dom_select_term->hasAttribute('value')) {
      break;
    }
    unset($dom_select_term);
  }
  if (empty($dom_select_term)) {
    school_crawl_logf($school_crawl_log, 4, "Unable to find the form input value associated with the %s semester.", $semester);
    return 1;
  }

  $semester_form = array();
  $term = $dom_select_term->getAttribute('value');
  $semester_form['term'] = $term;
  $uri = school_crawl_url($uri, 'data.php');
  $sections_json = school_crawl_geturi($uri, $cookies, $school_crawl_log, $semester_form);

  // Responds with JSON.
  if (empty($sections_json)) {
    school_crawl_logf($school_crawl_log, 2, "Nothing returned for listing term %s.", $term);
  }
  $sections = json_decode($sections_json, TRUE);
  if ($sections === NULL) {
    school_crawl_logf($school_crawl_log, 2, "Unable to parse JSON: %s.", json_last_error_msg());
    return 1;
  }
  school_crawl_logf($school_crawl_log, 7, "Type of JSON: %s.", gettype($sections));
  if (is_array($sections)) {
    school_crawl_logf($school_crawl_log, 7, "Keys in root: %s", implode(', ', array_keys($sections)));
  }
  foreach (['term', 'data'] as $key) {
    if (!array_key_exists($key, $sections)) {
      school_crawl_logf($school_crawl_log, 1, "JSON missing key: %s", $key);
      return 1;
    }
    $value = $sections[$key];
    school_crawl_logf($school_crawl_log, 7, "Type of JSON.%s: %s.", $key, gettype($value));
    if (is_array($value)) {
      school_crawl_logf($school_crawl_log, 7, "First 10 keys in JSON.%s: %s", $key, implode(', ', array_slice(array_keys($value), 0, 10)));
    }
  }
  $data = $sections['data'];
  if (!is_array($data)) {
    school_crawl_logf($school_crawl_log, 1, "Expected array in JSON.data. Got %s.", gettype($data));
    return 1;
  }
  foreach ($data as $row) {
    school_crawl_logf($school_crawl_log, 7, "Keys JSON.data[0]: %s", implode(', ', array_keys($row)));
    school_crawl_logf($school_crawl_log, 8, "First row: %s", json_encode($row));
    break;
  }

  $state = array(
    'semester' => $semester,
    'found_multiple_meettimes_example' => FALSE,
    'school_crawl_log' => &$school_crawl_log,
    'timeZone' => new DateTimeZone('America/Detroit'),
  );
  foreach ($data as $row) {
    _hope_crawl_semester_json_row($state, $row);
  }
}

function _hope_crawl_semester_json_row(&$state, $row) {
  // Examples of rows:
  //
  // Simple, SCBCRSE_TITLE is the only title provided, so should fall back to that.
  // {"SSBSECT_TERM_CODE":"202101","CRSE_STATUS":"O","SSRSYLN_LONG_COURSE_TITLE":null,"SSBSECT_CRSE_TITLE":null,"SCBCRSE_TITLE":"Basic Drawing","SSBSECT_SUBJ_CODE":"ART","STVSUBJ_DESC":"Art and Art History","SSBSECT_CRSE_NUMB":"114","SSBSECT_SEQ_NUMB":"02","SSBSECT_CRN":"11001","SSBSECT_CREDIT_HRS":null,"SCBCRSE_CREDIT_HR_HIGH":null,"SCBCRSE_CREDIT_HR_IND":null,"SCBCRSE_CREDIT_HR_LOW":"2","ATTR_CODES":"FA2","MEETTIMES":[{"sunday":"","monday":"","tuesday":"T","wednesday":"","thursday":"R","friday":"","saturday":"","starttime":"0800","endtime":"1050","building":"DPRART","room":"131","startdate":"01\/11","enddate":"05\/07"}],"MEETDAYS":"TR","SIRASGN_PIDM":"30955","FACULTY_NAME":"Staff, TBA","SSBSECT_ENRL":"0","SSBSECT_MAX_ENRL":"17","SSBSECT_SCHD_CODE":"U","SEATS_AVAIL":"17","SSBSECT_WAIT_COUNT":"0","SSBSECT_PTRM_CODE":"1","SSBSECT_CAMP_CODE":"M","STVCAMP_DESC":"Main Campus","SSBSECT_PTRM_END_DATE":"07-MAY-21","SSBSECT_PTRM_START_DATE":"11-JAN-21","SSBSECT_PTRM_WEEKS":"16","COMMENT":null}
  //
  // {"SSBSECT_TERM_CODE":"202008","CRSE_STATUS":"O","SSRSYLN_LONG_COURSE_TITLE":"CH1, Hist\/Lit\/Phil: Real Life and the Good Life from Classical Times to Christian","SSBSECT_CRSE_TITLE":null,"SCBCRSE_TITLE":"Cultural Heritage I","SSBSECT_SUBJ_CODE":"IDS","STVSUBJ_DESC":"Interdisciplinary","SSBSECT_CRSE_NUMB":"171","SSBSECT_SEQ_NUMB":"06","SSBSECT_CRN":"81088","SSBSECT_CREDIT_HRS":null,"SCBCRSE_CREDIT_HR_HIGH":null,"SCBCRSE_CREDIT_HR_IND":null,"SCBCRSE_CREDIT_HR_LOW":"4","ATTR_CODES":"CH1,GLI","MEETTIMES":[{"sunday":"","monday":"M","tuesday":"","wednesday":"W","thursday":"","friday":"","saturday":"","starttime":"1400","endtime":"1520","building":"LUBBRS","room":"121","startdate":"09\/01","enddate":"12\/18"}],"MEETDAYS":"MW","SIRASGN_PIDM":"1880","FACULTY_NAME":"LaPorte, Joseph F.","SSBSECT_ENRL":"0","SSBSECT_MAX_ENRL":"22","SSBSECT_SCHD_CODE":"L","SEATS_AVAIL":"22","SSBSECT_WAIT_COUNT":"0","SSBSECT_PTRM_CODE":"1","SSBSECT_CAMP_CODE":"M","STVCAMP_DESC":"Main Campus","SSBSECT_PTRM_END_DATE":"18-DEC-20","SSBSECT_PTRM_START_DATE":"01-SEP-20","SSBSECT_PTRM_WEEKS":"16","COMMENT":"Topic: Real Life and the Good Life from Classical Times to Christian. Engl 113 highly recommended prior to this course."}
  //
  // Multiple meeting times:
  //
  // {"SSBSECT_TERM_CODE":"202101","CRSE_STATUS":"R","SSRSYLN_LONG_COURSE_TITLE":null,"SSBSECT_CRSE_TITLE":null,"SCBCRSE_TITLE":"Wind Ensemble","SSBSECT_SUBJ_CODE":"MUS","STVSUBJ_DESC":"Music","SSBSECT_CRSE_NUMB":"130","SSBSECT_SEQ_NUMB":"01","SSBSECT_CRN":"11015","SSBSECT_CREDIT_HRS":"1","SCBCRSE_CREDIT_HR_HIGH":"1","SCBCRSE_CREDIT_HR_IND":"OR","SCBCRSE_CREDIT_HR_LOW":"0","ATTR_CODES":"FA2","MEETTIMES":[{"sunday":"","monday":"","tuesday":"","wednesday":"","thursday":"","friday":"F","saturday":"","starttime":"1300","endtime":"1430","building":"JHMILR","room":"130","startdate":"01\/11","enddate":"05\/07"},{"sunday":"","monday":"M","tuesday":"","wednesday":"W","thursday":"","friday":"","saturday":"","starttime":"1700","endtime":"1820","building":"JHMILR","room":"130","startdate":"01\/11","enddate":"05\/07"}],"MEETDAYS":"MWF","SIRASGN_PIDM":"36858","FACULTY_NAME":"Southard, Robert G.","SSBSECT_ENRL":"0","SSBSECT_MAX_ENRL":"30","SSBSECT_SCHD_CODE":"E","SEATS_AVAIL":"30","SSBSECT_WAIT_COUNT":"0","SSBSECT_PTRM_CODE":"1","SSBSECT_CAMP_CODE":"M","STVCAMP_DESC":"Main Campus","SSBSECT_PTRM_END_DATE":"07-MAY-21","SSBSECT_PTRM_START_DATE":"11-JAN-21","SSBSECT_PTRM_WEEKS":"16","COMMENT":"Written permission of department chair required. Please contact Dr. Southard for more information - southard@hope.edu."}
  //
  // Attempt to dissect the field names:
  //
  // SSBSECT_TERM_CODE: The term’s name. Also available in the
  //   response as the key term. Sent in the request as post form data
  //   value with name term. The presence of this value in the data
  //   makes it possible for data from multiple terms to be stored
  //   together in the same file, but that either isn’t a thing or,
  //   if Hope does return that, we wouldn’t want it because that
  //   would increase the chance of hitting the memory limits.
  //
  // CRSE_STATUS: The statuses are defined in the HTML as X=Cancelled,
  //   C=Closed, R=Permission(… required?), P=Completed, I=In
  //   progress, O=Open, W=Waitlisted.
  //
  // SSRSYLN_LONG_COURSE_TITLE: The most specific/long version of the
  //    title. Might be section-specific in the case of a super
  //    generic course title(?). Unsure about meaning of SSRYLN.
  //
  // SSBSECT_CRSE_TITLE: The second most specific/long version of the
  //    title. Based on the name, seems to be a section-specific
  //    title. If present, we should set the section title.
  //
  // SCBCRSE_TITLE: The course title. Most short/generalized.
  //
  // SSBSECT_SUBJ_CODE: Subject code (e.g., MUS for music).
  //
  // STVSUBJ_DESC: A human-readable description of the subject code
  //   (e.g., “Music” for MUS).
  //
  // SSBSECT_CRSE_NUMB: The course number.
  //
  // SSBSECT_SEQ_NUMB: The section.
  //
  // SSBSECT_CRN: The unique registration code(?), synonym.
  //
  // SSBSECT_CREDIT_HRS: Number of credit hours. Might vary per
  //   section in course.
  //
  // SCBCRSE_CREDIT_HR_HIGH: The maximum number of credit hours one
  //   might have when signing up for a particular section from this
  //   course. Is NULL if there is only one option for credit hous for
  //   the course. Slate Permutate already calculates this from
  //   knowing SSBSECT_CREDIT_HRS.
  //
  // SCBCRSE_CREDIT_HR_IND: The credit hours indicator? Weird. If TO,
  //   the number of credit hours for the course is considered a range
  //   from SCBCRSE_CREDIT_HR_LOW through SCBCRSE_CREDIT_HR_HIGH. If
  //   OR, the number of credit hours for the course is considered to
  //   be either the _LOW or the _HIGH. If NULL, _LOW is used and
  //   _HIGH is null (indicating only one choice for credit hours for
  //   the course). Probably just a human thing so that a course with
  //   2 different options for credit hours is shown as “X or Y”
  //   whereas a course with more than 2 different options for credit
  //   hours is just shown as a range.
  //
  //   Slate Permutate does not care about this. It can infer all of
  //   this information from analyzing SSBSECT_CREDIT_HRS stored in
  //   its Sections.
  //
  // SCBCRSE_CREDIT_HR_LOW: The minimum number of credit hours for the
  //   course or, if there is only one choice for an amount of credit
  //   hours for a course, NULL.
  //
  // ATTR_CODES: Some list of attributes. See the JavaScript for more
  //   information.
  //
  // MEETTIMES: An array of meeting time objects:
  //
  //   sunday, monday=M, tuesday=T, wednesday=W, thursday=R, friday=F,
  //     saturday: key will be set nonempty (to a particular day
  //     character, noted if known) if the meeting occurs on that day.
  //
  //   starttime: Empty string if the section has its times yet to be
  //     announced. Otherwise, 4 digits of the start time in 24-hour
  //     0-padded HHMM.
  //
  //   endtime: same format as starttime and rules regarding TBA, the
  //     end time.
  //
  //   building: The building (e.g., SCICTR)
  //
  //   room: The room number (e.g., 1228)
  //
  //   startdate: non-year date format 2-digit-zero-padded-month '/'
  //     2-digit-zero-padded-day-of-month. (e.g., “12/18”) Interpret
  //     as first matching greater than or equal to
  //     SSBSECT_PTRM_START_DATE which includes a year.
  //
  //   enddate: same format as startdate. Interpret as first matching
  //     less or equal to SSBSECT_PTRM_END_DATE.
  //
  // MEETDAYS: The string “TBA” if the times are yet to be
  //   announced. Might be “TBA/ONLINE”. Otherwise, a string of
  //   MTWRF (unknown values for Sunday and Saturday) matching the
  //   values used within MEETTIMES[n][dayOfWeekName]. A section-level
  //   summary. Slate Permutate just uses meetings directly, so
  //   useless.
  //
  // SIRASGN_PIDM: A numeric value such as 56640 or 421. Unreferenced
  //   by JavaScript.
  //
  // FACULTY_NAME: faculty name, like “Lunn, John E.”.
  //
  // SSBSECT_ENRL: Labeled “Act:” in the web interface. Sounds like
  //   the count of enrolled students.
  //
  // SSBSECT_MAX_ENRL: Labeled “Cap:” in the web interface. Likely
  //   the maximum number of students who may sign up for the section.
  //
  // SSBSECT_SCHD_CODE: Unknown. Unused in JavaScript.
  //
  // SEATS_AVAIL: Labeled “Rem:” (Remaining) in web interface. Means
  //   number of students who may still sign up.
  //
  // SSBSECT_WAIT_COUNT: Labeled “Wait:” in web interface. Probably
  //   count of people who are waitlisted.
  //
  // SSBSECT_PTRM_CODE: Part of term. 1 means full term, 2 means first
  //   half of term, 3 means second half of term.
  //
  // SSBSECT_CAMP_CODE: Campus. M=Main Campus, D=Domestic Off Campus,
  //   S=Study Abroad/Off Campus.
  //
  // STVCAMP_DESC: The human-readable descxription of the code given
  //   by SSBSECT_CAMP_CODE.
  //
  // SSBSECT_PTRM_END_DATE: e.g., 21-OCT-20. End of the part of the
  //   term that the section is in.
  //
  // SSBSECT_PTRM_START_DATE: Same format as
  //   SSBSECT_PTRM_END_DATE. Start of the part of the term that the
  //   section is in.
  //
  // SSBSECT_PTRM_WEEKS: Number of weeks in the part of the term that
  //   the section is in.
  //
  // COMMENT: Human-readable comments.

  $school_crawl_log =& $state['school_crawl_log'];
  $found_multiple_meettimes_example =& $state['found_multiple_meettimes_example'];
  $semester = $state['semester'];
  $timeZone = $state['timeZone'];

  // Multiple fields might provide title-like information. The
  // first-encountered title is set as the section title and the last
  // encountered title is set as the course title. This supports
  // “Cultural Heritage I” course with sections that are various
  // different topics.
  //
  // Precedence is defined by the JavaScripts at
  // https://schedule.hope.edu/ which just displays the most specified
  // title.
  $sectionTitle = NULL;
  $title = NULL;
  foreach (array(
    'SSRSYLN_LONG_COURSE_TITLE',
    'SSBSECT_CRSE_TITLE',
    'SCBCRSE_TITLE',
  ) as $titleKey) {
    $value = trim($row[$titleKey]);
    if (!empty($value)) {
      if (empty($sectionTitle)) {
        $sectionTitle = $value;
      }
      $title = $value;
    }
  }
  // If the same, omit the section title.
  if ($sectionTitle === $title) {
    $sectionTitle = NULL;
  }

  $subject_id = $row['SSBSECT_SUBJ_CODE'];
  $course_id = $row['SSBSECT_CRSE_NUMB'];
  $section_id = $row['SSBSECT_SEQ_NUMB'];

  if ($row['MEETDAYS'] === 'TBA' || $row['MEETDAYS'] === 'TBA/ONLINE') {
	  $semester->class_add(new Course($subject_id . '-' . $course_id, $title));
	  school_crawl_logf($school_crawl_log, 8, "Course %s-%s-%s has a section meeting with a TBA time, adding dummy course.", $subject_id, $course_id, $section_id);
    return;
	}

  $meetTimes = $row['MEETTIMES'];
  if (count($meetTimes) > 1 && !$found_multiple_meettimes_example) {
    $found_multiple_meettimes_example = TRUE;
    school_crawl_logf($school_crawl_log, 7, "Example of row with multiple meeting times: %s", json_encode($row));
  }

  $semesterPartOfTermStartDate = DateTime::createFromFormat('!d-M-y', $row['SSBSECT_PTRM_START_DATE'], $timeZone);
  $semesterPartOfTermEndDate = DateTime::createFromFormat('!d-M-y', $row['SSBSECT_PTRM_END_DATE'], $timeZone)->add(new DateInterval('P1D'));

  $synonym = $row['SSBSECT_CRN'];
  $instructor = $row['FACULTY_NAME'];

  foreach ($meetTimes as $meetTime) {
  $days = school_crawl_days_format($school_crawl_log, array_keys(array_filter(array_intersect_key($meetTime, array(
    'sunday' => TRUE,
    'monday' => TRUE,
    'tuesday' => TRUE,
    'wednesday' => TRUE,
    'thursday' => TRUE,
    'friday' => TRUE,
    'saturday' => TRUE,
  )), '_hope_crawl_days_filter')));
  $time_start = $meetTime['starttime'];
  $time_end = $meetTime['endtime'];
  $location = $meetTime['building'] . ' ' . $meetTime['room'];
  if (strlen($time_start) != 4 || strlen($time_end) != 4) {
	  school_crawl_logf($school_crawl_log, 4, "Section meeting (synonym=%s) has invalidly-formatted start time (%s) or end time (%s). Skipping.", $synonym, $time_start, $time_end);
	  continue;
	}

  $rawMeetTimeStartDate = $meetTime['startdate'];
  $rawMeetTimeEndDate = $meetTime['enddate'];
  $meetTimeDateRegex = ',^0*([0-9]+)/0*([0-9]+)$,';
  if (!preg_match($meetTimeDateRegex, $rawMeetTimeStartDate, $startDateMatches)
      || !preg_match($meetTimeDateRegex, $rawMeetTimeEndDate, $endDateMatches)) {
    school_crawl_logf($school_crawl_log, 4, "Section meeting (synonym=%s) has invalidly-formatted start date (%s) or end date (%s). Skipping.", $synonym, $rawMeetTimeStartDate, $rawMeetTimeEndDate);
    continue;
  }

  $startDate = new DateTime('now', $timeZone);
  $startDate->setTime(0, 0);
  $startDate->setDate($semesterPartOfTermStartDate->format('Y'), $startDateMatches[1], $startDateMatches[2]);
  // Since we only get the month and day of month for this, we might
  // be in the wrong year. Increment until we are equal to or greater
  // than the semester’s start date which does have a year.
  while ($startDate < $semesterPartOfTermStartDate) {
    $startDate->add(new DateInterval('P1Y'));
  }

  $endDate = new DateTime('now', $timeZone);
  $endDate->setTime(0, 0);
  $endDate->setDate($semesterPartOfTermEndDate->format('Y'), $endDateMatches[1], $endDateMatches[2]);
  $endDate->add(new DateInterval('P1D'));
  // Since we only get the month and day of month for this, we might
  // be in the wrong year. Decrement until we are equal to or less
  // than the semester’s end date which does have a year.
  while ($endDate > $semesterPartOfTermEndDate) {
    $endDate->sub(new DateInterval('P1Y'));
  }
  // Try to avoid creating garbage data…
  while ($endDate < $startDate) {
    school_crawl_logf($school_crawl_log, 2, "Calculated end date of %s is prior to calculated start date of %s for meeting of synonym=%s", $endDate->format('r'), $startDate->format('r'), $synonym);
    $endDate = $startDate;
    $endDate->add(new DateInterval('P1W'));
  }

  // Guessing the type of section_meeting: “attribute” of NSL
  // seems to be associated with labs. Matches `lab', `lab.', `
  // lab', ` labo'..., etc.
  $type = 'lecture';
  if (preg_match('/(^|[^a-z])lab($|o|[^a-z])/i', $title)) {
    $type = 'lab';
  }

  $section_meeting = new SectionMeeting(
    $days,
    $time_start,
    $time_end,
    $location,
    $type,
    $instructor,
    $startDate->getTimestamp(),
    $endDate->getTimestamp());
  $semester->section_meeting_add(
    $subject_id,
    $course_id,
    $title,
    $section_id,
    $synonym,
    $section_meeting,
    $type,
    $row['SSBSECT_CREDIT_HRS'],
    $sectionTitle);
  }
}
