<?php /* -*- mode: php; -*- */
/*
 * Copyright 2015 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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
 * A crawler for the Ellucian Company L.P. and affiliates’ software
 * based on oracle application server.
 */

$incdir = dirname(__FILE__) . DIRECTORY_SEPARATOR;
require_once $incdir . 'class.semester.inc';
require_once $incdir . 'class.course.inc';
require_once $incdir . 'class.section.php';
require_once $incdir . 'class.section_meeting.inc';

define('_SCHOOL_CRAWL_ORACLE_START_FORM', 'bwckschd.p_disp_dyn_sched');

function _school_crawl_oracle_common_prep(
  array &$school_crawl_log,
  array &$school,
  array &$options,
  &$curlsetup_hook,
  &$data_resolver)
{
  $school += array('oracle_uri' => $school['url'] . 'PROD/' . _SCHOOL_CRAWL_ORACLE_START_FORM);
  $options += array(
    'curlsetup_hook' => '_school_crawl_oracle_curlhook_ssl3',
    'data_resolver' => new SchoolCrawlOracleDataResolver($school_crawl_log),
  );
  $curlsetup_hook = $options['curlsetup_hook'];
  $data_resolver = $options['data_resolver'];
}

function _school_crawl_oracle_curlhook_ssl3(&$curl)
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

/*
 * \brief
 *   Crawl the list of semesters available from an oracle-compatible
 *   school.
 *
 * \param $school
 *   The school’s info array/handle.
 * \param $semesters
 *   The array to populate with various semesters available at this
 *   college.
 * \param $school_crawl_log
 *   The school_crawl_log handle.
 * \param $options
 *   An array with optional arguments 'curlsetup_hook', a function
 *   called on curl handles before use.  May also include key
 *   data_resolver which should be an instance of
 *   SchoolCrawlOracleDataResolver.
 */
function school_crawl_oracle_semester_list(array $school, array &$semesters, &$school_crawl_log, array $options = array())
{
  _school_crawl_oracle_common_prep(
    $school_crawl_log,
    $school,
    $options,
    $curlsetup_hook,
    $data_resolver);

  $cookies = array();

  $uri = $school['oracle_uri'];
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML(school_crawl_geturi($uri, $cookies, $school_crawl_log, NULL, TRUE, $curlsetup_hook));
  $semesters_select_node = $semesters_dom->getElementById('term_input_id');
  if ($semesters_select_node === NULL)
    {
      school_crawl_logf($school_crawl_log, 0, "Could not get list of available semesters to choose from.");
      return 1;
    }

  foreach ($semesters_select_node->childNodes as $semesters_option_node)
    {
      $semester_text = $semesters_option_node->textContent;
      $semester_value = $semesters_option_node->getAttribute('value');
      /* skip the empty or non-numeric “None” semester */
      if (empty($semester_value)
	  || !is_numeric($semester_value))
	continue;

      if (stripos($semester_text, 'continuing') !== FALSE)
	/* skip the year-long semesters dedicated to continuing education */
	continue;

      list($semester_season, $semester_year) = explode(' ', $semester_text);
      $semester_year = trim($semester_year);

      /* the college has two separate summer sessions, so distinguish between them */
      if (preg_match(';session ([0-9]+);i', $semester_text, $matches))
	$semester_season .= '_' . $matches[1];

      $semesters[] = new Semester($semester_year, strtolower($semester_season));
    }

  return 0;
}

function school_crawl_oracle_crawl_semester($school, $semester, &$school_crawl_log, array $options = array())
{
  _school_crawl_oracle_common_prep(
    $school_crawl_log,
    $school,
    $options,
    $curlsetup_hook,
    $data_resolver);

  $cookies = array();
  $uri = $school['oracle_uri'];
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML(school_crawl_geturi($uri, $cookies, $school_crawl_log, NULL, TRUE, $curlsetup_hook));
  $semesters_select_node = $semesters_dom->getElementById('term_input_id');
  if (empty($semesters_select_node))
    {
      school_crawl_logf($school_crawl_log, 0, "Could not locate the list of semesters from which to choose.");
      return 1;
    }

  $semesters_form = school_crawl_element_ancestor($semesters_select_node, 'form');
  if ($semesters_form === NULL)
    {
      school_crawl_logf($school_crawl_log, 0, "Unable to find <form /> associated with semester.");
      return 1;
    }
  $semesters_post = school_crawl_form($semesters_form);

  $semester_found = FALSE;
  foreach ($semesters_select_node->childNodes as $semesters_option_node)
    {
      $semester_text = $semesters_option_node->textContent;
      $semester_value = $semesters_option_node->getAttribute('value');
      /* skip the empty or non-numeric “None” semester */
      if (empty($semester_value)
	  || !is_numeric($semester_value))
	continue;

      school_crawl_logf($school_crawl_log, 5, "Parsing semester_text=%s", $semester_text);
      list($semester_season, $semester_year) = explode(' ', $semester_text);
      $semester_year = trim($semester_year);

      /* the college has two separate summer sessions, so distinguish between them */
      if (preg_match(';session ([0-9]+);i', $semester_text, $matches))
	$semester_season .= '_' . $matches[1];
      $semester_season = strtolower($semester_season);

      if ($semester_year == $semester->year_get()
	  && $semester_season == $semester->season_get())
	{
	  $semester_found = TRUE;
	  break;
	}
    }
  if (!$semester_found)
    {
      school_crawl_logf($school_crawl_log, 1, "Unable to find the entry for semester %s.", $semester);
      return 1;
    }

  $semesters_post[$semesters_select_node->getAttribute('name')] = $semester_value;

  $subjects_dom = new DOMDocument();
  $uri = school_crawl_url($uri, $semesters_form->getAttribute('action'));
  $subjects_dom->loadHTML(school_crawl_geturi($uri, $cookies, $school_crawl_log, $semesters_post, TRUE, $curlsetup_hook));

  $subjects_form_nodelist = $subjects_dom->getElementsByTagName('form');
  if (!$subjects_form_nodelist->length)
    {
      school_crawl_logf($school_crawl_log, 0, "Unable to find <form /> to submit for the subjects-choosing page.");
      return 1;
    }
  $subjects_form_node = $subjects_form_nodelist->item(0);
  $subjects_post = school_crawl_form($subjects_form_node);

  $subjects_select_node = $subjects_dom->getElementById('subj_id');
  $subjects_select_name = $subjects_select_node->getAttribute('name');
  $subjects_select_found_all = FALSE;
  foreach ($subjects_select_node->childNodes as $subjects_option_node)
    if (!strcasecmp('all', trim($subjects_option_node->textContent)))
      {
	$subjects_post[$subjects_select_name][] = $subjects_option_node->getAttribute('value');
	$subjects_select_found_all = TRUE;
      }
  /*
   * Support there being no “all”—instead, just actually select
   * everything ^^. Though, truly, I think the oracle server can
   * handle smaller chunks better than the ridiculousness of trying to
   * return everything at once.
   */
  if (!$subjects_select_found_all)
    foreach ($subjects_select_node->childNodes as $subjects_option_node)
      $subjects_post[$subjects_select_name][] = $subjects_option_node->getAttribute('value');

  $courses_dom = new DOMDocument();
  $uri = school_crawl_url($uri, $subjects_form_node->getAttribute('action'));
  $courses_dom->loadHTML(school_crawl_geturi($uri, $cookies, $school_crawl_log, $subjects_post, TRUE, $curlsetup_hook));

  $courses_xpath = new DOMXPath($courses_dom);

  /* Find the first subtable with at least a section listing in it. */
  foreach ($courses_xpath->query('//table[@class="datadisplaytable"]/tr[th[@class="ddheader"]]') as $tr_header_node)
    break;
  if (empty($tr_header_node))
    {
      school_crawl_logf($school_crawl_log, 0, "Unable to find the row of the course/section data table which gives us the mappings of column names onto columns.");
      return 1;
    }

  $section_getters = array();
  foreach (
    array(
      'registration_number',
      'section_id',
      'credits',
      'dates',
      'title',
      'days',
      'times',
      'instructor',
      'location',
    ) as $column_key)
    $section_getters[$column_key] = $data_resolver->resolve($column_key, $tr_header_node);

  /* error check */
  foreach ($section_getters as $name => $section_getter)
    {
      if (empty($section_getter))
	{
	  school_crawl_logf(
	    $school_crawl_log, 0, "Unable to find column offset for “%s”.",
	    $name);
	  return 1;
	}
      else
	school_crawl_logf($school_crawl_log, 9, "%s → %s", $name, $section_getter);
    }

  $row_i = -1;
  foreach ($courses_xpath->query('//table[@class="datadisplaytable" and tr/th[@class="ddheader"]]/tr[not(th[@class="ddheader"])]') as $tr_node)
    {
      $row_i++;
      school_crawl_logf($school_crawl_log, 9, "Found a row.");
      $children = school_crawl_table_rownodes($tr_node);

      $suitable = TRUE;
      foreach ($section_getters as $key => $section_getter)
	if (!$section_getter->valid($tr_node, $children))
	  {
	    school_crawl_logf($school_crawl_log, 0, "Row %d unsuitable for key %s", $row_i, $key);
	    $suitable = FALSE;
            return 1;
	  }
      if (!$suitable)
	/*
	 * Skip this row because it isn’t suitable for reading data
	 * from.
	 */
	continue;

      /*
       * There are some rows with the time set to TBA and with empty
       * section_id columns. Respond to this by skipping empty
       * section_id columns since there's no useful data in these
       * rows. We use strlen() < 3 because trim() doesn't take care of
       * &nbsp; :-/
       *
       * There are other times that the section_id row is empty and
       * the time column is set to something. In this case, the
       * subsequent rows are describing additional SectionMeetings
       * which should be added to the existing Section.
       */
      $section_id = trim($section_getters['section_id']->get($tr_node, $children));
      if (strlen($section_id) > 2)
	{
	  school_crawl_logf($school_crawl_log, 8, "Handling section %s.", $section_id);
	  /**
	   * \todo
	   *   If a section's section ID ends in `W', like `EFW', that
	   *   means it's a semi-online course. We should probably
	   *   distinguish these from normal sections, probably
	   *   disabling them from showing up by default.
	   *   —ccbcmd specific probably!
	   */
	  $section_id_parts = Section::parse($section_id);
	  $registration_number = $section_getters['registration_number']->get($tr_node, $children);
	  $credit_hours = (float)$section_getters['credits']->get($tr_node, $children);
	  $section = new Section($section_id_parts['section'], array(), $registration_number, $credit_hours);
	  $semester->section_add($section_id_parts['department'], $section_id_parts['course'], $section,
				 trim($section_getters['title']->get($tr_node, $children)));
	}
      if (empty($section))
	{
	  school_crawl_logf($school_crawl_log, 4, "Expected a section row before having a row with only partial data. Ignoring row.");
	  continue;
	}

      $instructor = $section_getters['instructor']->get($tr_node, $children);

      {
	$time_range_text = $section_getters['times']->get($tr_node, $children);
	if (strpos($time_range_text, 'TBA') !== FALSE)
	  /*
	   * There is no way to get meeting info and create
	   * SectionMeetings.
	   */
	  continue;

	if (($dash_pos = strpos($time_range_text, '-')) === FALSE)
	  {
	    school_crawl_logf($school_crawl_log, 0, "Unable to understand course's time range format, cannot find dash: ``%s''.",
		    $time_range_text);
	    return 1;
	  }

	$time_start_text = trim(substr($time_range_text, 0, $dash_pos));
	$time_start = DateTime::createFromFormat('h:i a', $time_start_text, new DateTimeZone('UTC'));
	$time_end_text = trim(substr($time_range_text, $dash_pos + 1));
	/*
	 * Make sure that _only_ one date range is specified to ensure
	 * data integrity. I.e., make sure that the college doesn't
	 * suddenly support multiple meeting times in one field
	 * without our anticipating that and then cause us to have
	 * invalid data. ;-). The college does support multiple
	 * section meetings, it does this by having multiple rows per
	 * section. The extra rows _only_ have the days, time, prof,
	 * and dates columns. --binki
	 */
	if (strpos($time_end_text, '-') !== FALSE)
	  {
	    school_crawl_logf($school_crawl_log, 4, "Entry seems to have invalid date column data: ``%s'' time_end_text: ``%s''.",
		    $time_range_text, $time_end_text);
	    continue;
	  }
	$time_end = DateTime::createFromFormat('h:i a', $time_end_text, new DateTimeZone('UTC'));
	if ($time_end === FALSE || $time_start === FALSE)
	  {
	    school_crawl_logf($school_crawl_log, 4, "Error parsing start or end time: start: ``%s'' end: ``%s''.",
		    $time_start_text, $time_end_text);
	    continue;
	  }

	$days = school_crawl_days_str_format($school_crawl_log, $section_getters['days']->get($tr_node, $children));

	$section_dates = $section_getters['dates']->get($tr_node, $children);
	$date_start = $date_end = NULL;
	if (preg_match(';^([0-9]+)/([0-9]+)-([0-9]+)/([0-9]+)$;', $section_dates, $section_dates_matches))
	  {
	    $date_start = gmmktime(0, 0, 0, $section_dates_matches[1], $section_dates_matches[2], $semester->year_get());
	    $date_end = gmmktime(0, 0, 0, $section_dates_matches[3], $section_dates_matches[4], $semester->year_get());
	  }

	$section->meeting_add(
	  new SectionMeeting(
	    $days, school_crawl_time_format($time_start), school_crawl_time_format($time_end),
	    $section_getters['location']->get($tr_node, $children),
	    'lecture',
	    $instructor,
	    $date_start,
	    $date_end));
      }
    }

  return 0;
}

class SchoolCrawlOracleDataColumnGetter
{
  protected $school_crawl_log;

  public function __construct(array &$school_crawl_log, $column_i)
  {
    $this->school_crawl_log = &$school_crawl_log;
    $this->column_i = $column_i;
  }

  function access(DOMNodeList $children)
  {
    return school_crawl_table_rownode_index($children, $this->column_i);
  }

  public function get(DOMNode $tr_node, DOMNodeList $children)
  {
    return $this->access($children)->textContent;
  }

  /**
   * \brief
   *   Check whether or not this column getter can be used on a
   *   particular row.
   */
  public function valid(DOMNode $tr_node, DOMNodeList $children)
  {
    $value = $this->access($children);
    return TRUE
      /*
       * Does the index even point to something?
       */
      && !empty($value)
      /*
       * Any row where our data column turns out to be a header isn’t
       * good.
       */
      && strcmp($value->tagName, 'th')
      ;
  }

  public function __toString()
  {
    return 'c[' . $this->column_i . ']';
  }
}

class SchoolCrawlOracleDataResolver
{
  protected $school_crawl_log;

  /*
   * This default mapping is for ccbcmd. But as more oracle schools
   * are added, can move what seems uncommon out to ccbcmd itself.
   */
  protected $key_mapping = array(
    'registration_number' => 'CRN',
    'section_id' => 'subj/crse/sec',
    'credits' => 'credhrs',
    'dates' => 'sessiondates',
  );

  public function __construct(array &$school_crawl_log)
  {
    $this->school_crawl_log = &$school_crawl_log;
  }

  /**
   * \returns
   *   A SchoolCrawlOracleDataGetter that can be used to get that
   *   piece of data.
   */
  public function resolve($key, DOMElement $tr_header_node)
  {
    $this->key_mapping += array(
      $key => $key,
    );

    $column_i = school_crawl_table_resolve_column($tr_header_node, $this->key_mapping[$key]);
    error_log('thing req=' . $key . '→' . $this->key_mapping[$key] . '=' . $column_i);
    if ($column_i !== FALSE)
      return new SchoolCrawlOracleDataColumnGetter($this->school_crawl_log, $column_i);
  }
}
