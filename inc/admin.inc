<?php /* -*- mode: php; -*- */
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

$inc_dir = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;

require_once($inc_dir . 'schedule_store.inc');
require_once($inc_dir . 'class.semester.inc');
require_once($inc_dir . 'school.inc');
require_once($inc_dir . 'school.crawl.inc');

/**
 * \file
 *   Functions which are only needed when recreating the cache.
 */

/**
 * \brief
 *   Returns the list of available school IDs or NULL on error.
 */
function school_list()
{
  $schoold_dir_name = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'school.d';
  $schoold_dir = opendir($schoold_dir_name);
  if ($schoold_dir === FALSE)
    {
      fprintf(STDERR, "Unable to open school.d directory. Was using path: `%s'\n",
	      $schoold_dir_Name);
      return NULL;
    }

  $school_id_list = array();
  while ($filename = readdir($schoold_dir))
    {
      if (!preg_match('/^([a-z0-9]+)\.inc$/', $filename, $matches))
	continue;

      $school_id_list[] = $matches[1];
    }

  closedir($schoold_dir);

  return $school_id_list;
}

/**
 * \brief
 *   Compare the two schools by their names.
 *
 * \see strcmp()
 */
function school_cmp($school_a, $school_b)
{
  return strcmp($school_a['name'], $school_b['name']);
}

/**
 * \brief
 *   Write out the cache file which remembers the list of available
 *   schools.
 *
 * \param $schools
 *   An array of school handles.
 */
function school_cache($schools)
{
  $list_cache = array();
  $domain_cache = array();

  $cache_dir_name = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..'
    . DIRECTORY_SEPARATOR . 'cache' . DIRECTORY_SEPARATOR;
  $cache_auto_dir_name = $cache_dir_name . 'auto' . DIRECTORY_SEPARATOR;

  foreach ($schools as $school)
    {
      $list_cache[$school['id']] = array(
					 'name' => $school['name'],
					 'url' => $school['url'],
					 'crawled' => !empty($school['crawled']),
					 );
      foreach ($school['domains'] as $school_domain)
	{
	  $domain_cache_ptr =& $domain_cache;

	  $domain_parts = array_reverse(explode('.', $school_domain));
	  while (count($domain_parts) > 1)
	    {
	      $domain_part = array_shift($domain_parts);
	      if (!isset($domain_cache_ptr[$domain_part])
		  || !is_array($domain_cache_ptr[$domain_part]))
		$domain_cache_ptr[$domain_part] = array();
	      $domain_cache_ptr =& $domain_cache_ptr[$domain_part];
	    }
	  /*
	   * get the last part which is unambiguously identifies this
	   * school combined with the previous parts
	   */
	  $domain_part = array_shift($domain_parts);
	  $domain_cache_ptr[$domain_part] = $school['id'];
	}


      /*
       * autocomplete stuff -- per school
       *
       * We don't do anything if crawled_notreally is set because this
       * way we can get incremental crawling. Really useful if one's
       * just debugging one of the school crawling scripts and doesn't
       * want to run all crawlers ;-).
       */
      if (!empty($school['crawled']) && !isset($school['crawled_notreally']))
	{
	  $cache_auto_school_dir_name = $cache_auto_dir_name . $school['id'] . DIRECTORY_SEPARATOR;
	  if (!is_dir($cache_auto_school_dir_name))
	    {
	      if (!mkdir($cache_auto_school_dir_name, 0755, TRUE))
		error_log('Unable to create needed directory: `' . $cache_auto_dir_name . '\'');
	    }

	  $semesters = array();
	  $semester_weights = 0;
	  /*
	   * Try to presort the semesters into the proper order based
	   * on time_start/time_end. We want the older semesters to be
	   * nearer to the end of the list. This way, the crawler
	   * doesn't have to decide how to sort the semesters itself:
	   */
	  usort($semesters, 'school_cache_semesters_sort');

	  foreach ($school['crawled_semesters'] as $semester)
	    {
	      $semesters[$semester->id()] = array(
						  'id' => $semester->id(),
						  'time_start' => $semester->time_start_get(),
						  'time_end' => $semester->time_end_get(),
						  'weight' => $semester_weights ++,
						  'name' => $semester->name_get(),
						  );
	    } /* foreach ( => $semester) */
	  /*
	   * Store/cache the semester metadata:
	   */
	  $semesters_file = fopen($cache_auto_school_dir_name . '-semesters', 'wb');
	  fwrite($semesters_file, serialize($semesters));
	  fclose($semesters_file);
	}
    }
  uasort($list_cache, 'school_cmp');

  $cache = array('list' => $list_cache, 'domains' => $domain_cache);

  $cache_file_name =  $cache_dir_name . 'schools';
  $cache_file = fopen($cache_file_name, 'wb');
  if ($cache_file === FALSE)
    {
      fprintf(STDERR, "Unable to open `%s' for writing\n",
	      $cache_file_name);
      return 1;
    }
  fwrite($cache_file, serialize($cache));
  fclose($cache_file);

  return 0;
}

/**
 * \brief
 *   To support usort() call in school_cache() to sort semesters.
 */
function school_cache_semesters_sort (Semester $sem_a, Semester $sem_b)
{
  return $sem_a->time_start_get() - $sem_b->time_start_get();
}

/**
 * \brief
 *   Invoke a school's registration data crawler.
 *
 * Each school may export registration data on publically accessible
 * websites. Thus, we populate some autocomplete information by
 * crawling these pages and storing the information in a special set
 * of caches.
 *
 * Because crawling code can be non-trivial, it should be separated
 * from a school's main .inc file. Thus, if a school supports
 * crawling, it will have a file called
 * schools.d/<school_id>.crawl.inc. In this file, a function called
 * <school_id>_crawl(array &$semesters, $verbosity = 1) must be
 * defined. It must accept at least one argument, the array to be
 * filled with Semester objects. It must populate this array with
 * individual Semester objects and fill those with Course objects and
 * populate those courses with the sections with as much detail as
 * possible. This function may return 1 to indicate an error must
 * return 0 to indicate success.
 *
 * If the crawling is successful, a 'crawled' key is added to the
 * $school handle. school_cache() will use this to help indicate that
 * a school _has_ autocomplete information, which might affect the
 * appearance and JS stuff for the input.php page.
 *
 * \param $school
 *   The school which should be checked for crawl functionality and
 *   crawled.
 * \param $page
 *   The Page object for which HTML formatted logs should be outputted
 * \param $verbosity
 *   How verbose to be. Sane values are from 0 through 10.
 * \return
 *   A school_crawl_log handle, upopn which school_crawl_log_fetch()
 *   may be used.
 */
function school_crawl(array &$school, Page $page = NULL, $verbosity = 1)
{
  $cache_dir_name = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..'
    . DIRECTORY_SEPARATOR . 'cache' . DIRECTORY_SEPARATOR;
  $cache_auto_dir_name = $cache_dir_name . 'auto' . DIRECTORY_SEPARATOR;
  $cache_auto_school_dir_name = $cache_auto_dir_name . $school['id'] . DIRECTORY_SEPARATOR;
  if (!is_dir($cache_auto_school_dir_name))
    {
      if (!mkdir($cache_auto_school_dir_name, 0755, TRUE))
	error_log('Unable to create needed directory: `' . $cache_auto_dir_name . '\'');
    }

  if (empty($school['crawled']))
    $school['crawled'] = FALSE;
  $school['crawled_notreally'] = TRUE;

  $school_crawl_semesters_list_func = $school['id'] . '_crawl_semester_list';
  if (!function_exists($school_crawl_semesters_list_func))
    return;

  $school_crawl_log_opts = array('verbosity' => $verbosity);
  if (defined('STDERR'))
    $school_crawl_log_opts['stream'] = STDERR;
  if ($page !== NULL)
    $school_crawl_log_opts['page'] = $page;
  $school_crawl_log = school_crawl_log_init($school, $school_crawl_log_opts);

  $semesters = array();

  if ($verbosity > 0)
    school_crawl_logf($school_crawl_log, 2, "Calling %s()...", $school_crawl_semesters_list_func);

  $ret = $school_crawl_semesters_list_func($school, $semesters, $school_crawl_log);
  if ($ret)
    {
      school_crawl_logf($school_crawl_log, 1, "Crawling %s failed: %s() returned nonzero",
			$school['id'], $school_crawl_semesters_list_func);
      school_crawl_logf($school_crawl_log, 6, "");
      return;
    }

  $successful_semesters = array();
  $school_crawl_semester_func = $school['id'] . '_crawl_semester';
  if (!function_exists($school_crawl_semester_func))
    {
      school_crawl_logf($school_crawl_log, 3, "%s() is defined but %s() isn't.",
			$school_crawl_semesters_list_func, $school_crawl_semester_func);
      return;
    }

  foreach ($semesters as $semester)
    {
      school_crawl_logf($school_crawl_log, 2, "Calling %s(%s)...", $school_crawl_semester_func, $semester);
      $ret = $school_crawl_semester_func($school, $semester, $school_crawl_log);
      if ($ret)
	{
	  school_crawl_logf($school_crawl_log, 1, "Failed to crawl semester %s. Skipping semester.", $semester);
	  continue;
	}

      /*
       * Write out this semester's cache now that we're here.
       */
      $cache_auto_school_semester_dir_name = $cache_auto_school_dir_name . $semester->id() . DIRECTORY_SEPARATOR;
	      if (!is_dir($cache_auto_school_semester_dir_name))
		{
		  if (!mkdir($cache_auto_school_semester_dir_name, 0755, TRUE))
		    error_log('Unable to create needed directory: `' . $cache_auto_school_semester_dir_name . '\'');
		}

	      $departments = $semester->departments_get();
	      sort($departments);

	      /*
	       * Create friendly department labels (like `MATH
	       * (Mathematics)') if the crawler has given us that
	       * data.
	       */
	      $has_department_names = FALSE;
	      $department_names = array();
	      foreach ($semester->department_names_get() as $dept => $dept_name)
		{
		  $department_names[] = array('value' => $dept, 'label' => $dept . ' (' . $dept_name . ')');
		  if (!$has_department_names && strcmp($dept, $dept_name))
		    $has_department_names = TRUE;
		}

	      $dept_file = fopen($cache_auto_school_semester_dir_name . '-depts', 'wb');
	      fwrite($dept_file, serialize($has_department_names ? $department_names : $departments));
	      fclose($dept_file);

	      /* now per-department autocomplete */
	      foreach ($departments as $department)
		{
		  $classes = $semester->department_classes_get($department);
		  $courses_json = array();
		  foreach ($classes as $course_id)
		    {
		      $course = $semester->class_get($department, $course_id);
		      $course_json = array(
			'value' => $course->getName(),
			'label' => $course->getName(),
		      );
		      if (strlen($course_title = $course->title_get()))
			$course_json['label'] .= ' (' . $course_title . ')';
		      $courses_json[] = $course_json;
		    }

		  $classes_file = fopen($cache_auto_school_semester_dir_name . $department . '.sects', 'wb');
		  fwrite($classes_file, serialize($courses_json));
		  fclose($classes_file);

		  /* now individual section informations, pre-JSON-ized */
		  foreach ($classes as $class)
		    {
		      if (!is_dir($cache_auto_school_semester_dir_name . $department))
			mkdir($cache_auto_school_semester_dir_name . $department);
		      $class_file = fopen($cache_auto_school_semester_dir_name . $department . DIRECTORY_SEPARATOR . $class, 'wb');
		      fwrite($class_file, json_encode($semester->class_get($department, $class)->to_json_array()));
		      fclose($class_file);
		    }
		}

      /* Purge the data written to disk from memory */
      $semester->purge();

      school_crawl_logf($school_crawl_log, 6, "");
      $successful_semesters[] = $semester;
    }

  $school['crawled'] = TRUE;
  unset($school['crawled_notreally']);
  $school['crawled_semesters'] = $successful_semesters;

  return $school_crawl_log;
}


/**
 * \brief
 *   Recreate/update the school section autocomplete cache.
 *
 * \param $crawl_only
 *   If non-NULL, an array of school_ids to limit the cache recreation
 *   to. Useful for when developing a certain school's crawling
 *   function.
 * \param $verbosity
 *   An integer indicating how loud to be.
 */
function school_cache_recreate($crawl_only = NULL, Page $page = NULL, $verbosity = 5)
{
  $school_id_list = school_list();
  if (!$school_id_list)
    {
      fprintf(STDERR, "error: Unable to load schools.\n");
      return 1;
    }

  if ($crawl_only !== NULL)
    foreach ($crawl_only as $crawl_only_school_id)
      if (!in_array($crawl_only_school_id, $school_id_list))
	{
	  fprintf(STDERR, "error: Invalid school_id specified for crawling: %s\n",
		  $crawl_only_school_id);
	  return 1;
	}

  /* hide libxml errors from the console: */
  libxml_use_internal_errors(TRUE);

  $schools = array();
  $old_school_cache = _school_cache_load();
  foreach ($school_id_list as $school_id)
    {
      $school = school_load($school_id, TRUE);
      if (!$school)
	{
	  fprintf(STDERR, "Error loading school with school_id=%s\n",
		  $school_id);
	  return 1;
	}

      if ($crawl_only === NULL || in_array($school['id'], $crawl_only))
	{
	  $school_crawl_log = school_crawl($school, $page, $verbosity);
	}
      else
	{
	  /*
	   * try to allow incremental crawling by not wiping out old
	   * data and preserving the cached $school['crawled'].
	   */
	  if ($old_school_cache && isset($old_school_cache['list'][$school['id']]))
	    {
	      $old_school = $old_school_cache['list'][$school['id']];
	      $school['crawled'] = FALSE;
	      if (isset($old_school['crawled']))
		$school['crawled'] = $old_school['crawled'];
	      if ($school['crawled'])
		$school['crawled_notreally'] = TRUE;
	    }
	}

      $schools[] = $school;
    }

  if (school_cache($schools))
    {
      fprintf(STDERR, "Error writing out school cache\n");
      return 1;
    }

  return 0;
}

/**
 * \brief
 *   Purge a range of saved_schedules.
 *
 * \param $schedule_store
 *   The schedule_store handle for which a range of saved schedules
 *   must be deleted.
 * \param $time_min
 *   The minimum unix timestamp for the range of schedules to be
 *   purged.
 * \param $time_max
 *   The maximum unix timestamp for the range of schedules to be
 *   purged or NULL for no limit.
 * \return
 *   FALSE on failure, an integer indicating the number of deleted
 *   saved_schedules on success.
 */
function schedule_store_purge_range($schedule_store, $time_min = 0, $time_max = NULL)
{
  global $admin_enable_purge;

  $schedule_id_max = schedule_store_getmaxid($schedule_store);
  $num_deleted = 0;

  if (!$admin_enable_purge)
    return FALSE;

  for ($schedule_id = 0; $schedule_id < $schedule_id_max; $schedule_id ++)
    {
      $filename = $schedule_store['dir'] . DIRECTORY_SEPARATOR . $schedule_id;
      if (!file_exists($filename))
	continue;

      $statbuf = stat($filename);
      if ($statbuf['ctime'] >= $time_min
	  && ($time_max === NULL || $statbuf['ctime'] <= $time_max))
	{
	  unlink($filename);
	  $num_deleted ++;
	}
    }

  return $num_deleted;
}

/**
 * \brief
 *   A small testsuite to help developers.
 *
 * \return
 *   Number of failures.
 */
function test()
{
  $ideal = array('department' => 'CS',
		 'course' => '262',
		 'section' => 'A');
  $ideal_c = $ideal;
  unset($ideal_c['section']);
  $n = 0;

  $t1 = 'CS-262-A';
  $n += assert_equal($t1, Section::parse($t1), $ideal);
  $n += assert_equal($t1 . '_class', Course::parse($t1), $ideal_c);
  $t2 = 'cs262 a';
  $n += assert_equal($t2, Section::parse($t2), $ideal);
  $n += assert_equal($t2 . '_class', Course::parse($t2), $ideal_c);
  $t3 = 'cs 262 a';
  $n += assert_equal($t3, Section::parse($t2), $ideal);
  $n += assert_equal($t3 . '_class', Course::parse($t3), $ideal_c);

  $ideal['course'] .= 'L';
  $ideal_c['course'] = $ideal['course'];

  $t1 = 'CS-262L-A';
  $n += assert_equal($t1, Section::parse($t1), $ideal);
  $n += assert_equal($t1 . '_class', Course::parse($t1), $ideal_c);
  $t2 = 'cs262l a';
  $n += assert_equal($t2, Section::parse($t2), $ideal);
  $n += assert_equal($t2 . '_class', Course::parse($t2), $ideal_c);
  $t3 = 'cs 262l a';
  $n += assert_equal($t3, Section::parse($t2), $ideal);
  $n += assert_equal($t3 . '_class', Course::parse($t3), $ideal_c);

  $csv = '1,2,3,4
1,2 ,3,4
1,"2,",3,4
"1
1",2,3,4
"""1""",2,3,4

4';
  $csv_parsed = array(
    array('1', '2', '3', '4'),
    array('1', '2 ', '3', '4'),
    array('1', '2,', '3', '4'),
    array("1\n1", '2', '3', '4'),
    array('"1"', '2', '3', '4'),
    array(''),
    array('4'),
  );

  $n += assert_equal('csv', school_crawl_csv_parse($csv, array('eof' => TRUE)), $csv_parsed);
  $n += assert_equal('csv_buffer', $csv, '');

  $csv_partial = '1,2
3';
  /*
   * Check partial parsing support; give a situation where we
   * supposedly don't have eof.
   */
  $n += assert_equal('csv_partial', school_crawl_csv_parse($csv_partial), array(array('1', '2')));
  $n += assert_equal('csv_partial_buffer', $csv_partial, '3');

  $section_meeting_a = new SectionMeeting('mwf', '1900', '1950', NULL, 'lecture', NULL, 1335063574 /* 2012-04-22 (sat) */, 1348282798 /* bit after 2012-09-21 (fri) */);
  $n += assert_equal('date_start_get+mon', $section_meeting_a->date_start_get(), 1335207600);
  $n += assert_equal('date_end_get+fri', $section_meeting_a->date_end_get(), 1348257000);

  $section_meeting_b = new SectionMeeting('mwf', '1900', '1950', NULL, 'lecture', NULL, 1335495574 /* 2012-04-27 (thur) */, 1348109998 /* bit after 2012-09-19 (wed) */);
  $n += assert_equal('date_start_get+thur', $section_meeting_b->date_start_get(), 1335553200);
  $n += assert_equal('date_end_get+wed', $section_meeting_b->date_end_get(), 1348084200);

  /* The two section meetings above should overlap */
  $n += assert_equal('section_meeting_collide', $section_meeting_a->conflictsWith($section_meeting_b), TRUE);

  /*
   * A third section meeting has the same time of day but starts the
   * day after secftion_meeting_b:
   */
  $section_meeting_c = new SectionMeeting('mwf', '1900', '1950', NULL, 'lecture', NULL, 1348109998 /* bit after 2012-09-19 (wed) */, 1354406400 /* bit after 2012-12-01 (wed) */);
  $n += assert_equal('section_meeting_collide_a', $section_meeting_a->conflictsWith($section_meeting_c), TRUE);
  $n += assert_equal('section_meeting_no_collide_b', $section_meeting_b->conflictsWith($section_meeting_c), FALSE);

  /*
   * If a section meeting doesn't specify an absolute start/end time,
   * it must always conflict.
   */
  $section_meeting_d = new SectionMeeting('mwf', '1900', '1950');
  $n += assert_equal('section_meeting_collide_d_a', $section_meeting_d->conflictsWith($section_meeting_a), TRUE);
  $n += assert_equal('section_meeting_collide_a_d', $section_meeting_a->conflictsWith($section_meeting_d), TRUE);
  $n += assert_equal('section_meeting_collide_d_b', $section_meeting_d->conflictsWith($section_meeting_b), TRUE);
  $n += assert_equal('section_meeting_collide_b_d', $section_meeting_b->conflictsWith($section_meeting_d), TRUE);
  $n += assert_equal('section_meeting_collide_d_c', $section_meeting_d->conflictsWith($section_meeting_c), TRUE);
  $n += assert_equal('section_meeting_collide_c_b', $section_meeting_c->conflictsWith($section_meeting_d), TRUE);

  return $n;
}

/**
 * \brief
 *   A reimplementation of a standard testsuite utility.
 *
 * \return
 *   TRUE if the test failed.
 */
function assert_equal($name, $a, $b)
{
  if (is_array($a))
    {
      $bad = FALSE;
      if (!is_array($b))
	{
	  fprintf(STDERR, "Test ``%s'' failed: \$a is an array while \$b isn't.\n",
		  $name);
	  return TRUE;
	}

      foreach ($a as $key => $val)
	if (!$bad && isset($b[$key]))
	  $bad = assert_equal($name . '[' . $key . ']', $a[$key], $b[$key]);
        elseif (!$bad)
	  {
	    $bad = TRUE;
	    fprintf(STDERR, "\$b is missing key ``%s'' which \$a has.\n",
		    $key);
	  }
	foreach ($b as $key => $val)
	  if (!$bad && isset($a[$key]))
	    $bad = assert_equal($name . '[' . $key . ']', $a[$key], $b[$key]);
	  elseif (!$bad)
	    {
	      $bad = TRUE;
	      fprintf(STDERR, "\$a is missing key ``%s'' which \$b has.\n",
		      $key);
	    }

      if ($bad)
	{
	  fprintf(STDERR, "Test ``%s'' failed, see previous error message\n",
		  $name);
	  return TRUE;
	}

      return FALSE;
    }
  elseif (is_array($b))
    {
      fprintf(STDERR, "Test ``%s'' failed: \$b is an array; \$a isn't.\n",
	      $name);
      return TRUE;
    }
  elseif ($a === $b
	  && !strcmp($a, $b))
    {
      return FALSE;
    }
  else
    {
      fprintf(STDERR, "Test ``%s'' failed: `%s' === `%s' => %s, strcmp() == %d\n",
	      $name, $a, $b, $a === $b ? 'TRUE' : 'FALSE', strcmp($a, $b));
      return TRUE;
    }
  return TRUE;
}
