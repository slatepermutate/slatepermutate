#!/usr/bin/env php
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
 * \file
 *
 * Runs through schools.d grabbing and caching data, such as the
 * school listing used for the ``choose your school list''.
 */

$inc_base = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;
require_once($inc_base . 'school.inc');
require_once($inc_base . 'school.crawl.inc');
require_once($inc_base . 'class.semester.inc');

return main($argc, $argv);

function main($argc, $argv)
{
  $crawl = TRUE;
  $crawl_semester_year = '2011';
  $crawl_semester_season = Semester::SEASON_SPRING;

  $opts = getopt('h', array('no-crawl', 'crawl-only:', 'help'));

  if (isset($opts['help']) || isset($opts['h']))
    {
      usage($argv[0]);
      return 0;
    }

  if (isset($opts['no-crawl']))
    $crawl = FALSE;
  if (isset($opts['crawl-only']))
    $crawl_only = split(',', $opts['crawl-only']);

  $school_id_list = school_list();
  if (!$school_id_list)
    return 1;

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

      if ($crawl
	  && (!isset($crawl_only) || in_array($school['id'], $crawl_only)))
	{
	  school_crawl($school, $crawl_semester_year, $crawl_semester_season);
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

/*
 * functions which are only needed when recreating the cache.
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
					 'crawled' => $school['crawled'],
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
      if ($school['crawled'] && !isset($school['crawled_notreally']))
	{
	  $semester = $school['crawled_semester'];

	  $cache_auto_school_dir_name = $cache_auto_dir_name . $school['id'] . DIRECTORY_SEPARATOR;
	  if (!is_dir($cache_auto_school_dir_name))
	    {
	      if (!mkdir($cache_auto_school_dir_name, 0777, TRUE))
		error_log('Unable to create needed directory: `' . $cache_auto_dir_name . '\'');
	    }

	  $departments = $semester->departments_get();
	  sort($departments);

	  $dept_file = fopen($cache_auto_school_dir_name . '-depts', 'wb');
	  fwrite($dept_file, serialize($departments));
	  fclose($dept_file);

	  /* now per-department autocomplete */
	  foreach ($departments as $department)
	    {
	      $classes = $semester->department_classes_get($department);
	      $classes_file = fopen($cache_auto_school_dir_name . $department . '.sects', 'wb');
	      fwrite($classes_file, serialize($classes));
	      fclose($classes_file);

	      /* now individual section informations, pre-JSON-ized */
	      foreach ($classes as $class)
		{
		  if (!is_dir($cache_auto_school_dir_name . $department))
		    mkdir($cache_auto_school_dir_name . $department);
		  $class_file = fopen($cache_auto_school_dir_name . $department . DIRECTORY_SEPARATOR . $class, 'wb');
		  fwrite($class_file, json_encode($semester->class_get($department, $class)->to_json_array()));
		  fclose($class_file);
		}
	    }
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
 * <school_id>_crawl($semester) must be defined. It must accept one
 * argument, the Semester object which defines the time of year for
 * which courses should be retrieved. It must populate this empty
 * Semester object with Course object and populate those courses with
 * the sections with as much detail as possible.
 *
 * If the crawling is successful, a 'crawl' key is added to the
 * $school handle. school_cache() will use this to help indicate that
 * a school _has_ autocomplete information, which might affect the
 * appearance and JS stuff for the input.php page.
 *
 * \param $school
 *   The school which should be checked for crawl functionality and
 *   crawled.
 * \param $semester_year
 *   The year of the semester for which we should grab data.
 * \param $semester_season
 *   The season of the year of the semester for which we should grab
 *   data.
 */
function school_crawl(&$school, $semester_year, $semester_season, $verbosity = 1)
{
  $school['crawled'] = FALSE;

  $school_crawl_func = $school['id'] . '_crawl';
  if (!function_exists($school_crawl_func))
    return;

  $semester = new Semester($semester_year, $semester_season);

  if ($verbosity > 0)
    fprintf(STDERR, "%s()\n", $school_crawl_func);
  $ret = $school_crawl_func($semester, $verbosity);
  if ($ret)
    {
      fprintf(STDERR, "Crawling %s failed: %s() returned nonzero\n",
	      $school['id'], $school_crawl_func);
      fwrite(STDERR, "\n");
      return;
    }
  $school['crawled'] = TRUE;
  $school['crawled_semester'] = $semester;

  if ($verbosity > 0)
    fwrite(STDERR, "\n");
}

/**
 * \brief
 *   Display CLI interface usage.
 */
function usage($progname)
{
  fprintf(STDERR, "Usage: %s [--no-crawl] [--crawl-only=<school_id1>,<school_id2>,...] [--help] [-h]\n"
	  . "\n"
	  . " -h, --help   Show this usage information and exit.\n"
	  . "\n"
	  . " --no-crawl   Disable crawling during this rehash but preserve\n"
	  . "              previous cached crawl data.\n"
	  . " --crawl-only Takes a comma-separated list of school_ids whose\n"
	  . "              registration systems should be crawled for autofill\n"
	  . "              data. Cached data from schools not listed is preserved\n",
	  $progname);
}
