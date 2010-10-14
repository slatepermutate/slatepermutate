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

require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR . 'school.inc');
return main($argc, $argv);

function main($argc, $argv)
{
  $school_id_list = school_list();
  if (!$school_id_list)
    return 1;

  $schools = array();
  foreach ($school_id_list as $school_id)
    {
      $school = school_load($school_id);
      if (!$school)
	{
	  fprintf(STDERR, "Error loading school with school_id=%s\n",
		  $school_id);
	  return 1;
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
 * \todo
 *   If the list of displayed schools is to be sorted, this is the
 *   place to do it.
 *
 * \param $schools
 *   An array of school handles.
 */
function school_cache($schools)
{
  $list_cache = array();
  $domain_cache = array();
  foreach ($schools as $school)
    {
      $list_cache[$school['id']] = array(
					 'name' => $school['name'],
					 'url' => $school['url'],
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
    }
  uasort($list_cache, 'school_cmp');

  $cache = array('list' => $list_cache, 'domains' => $domain_cache);


  $cache_file_name = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..'
    . DIRECTORY_SEPARATOR . 'cache' . DIRECTORY_SEPARATOR . 'schools';
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
