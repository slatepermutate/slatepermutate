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
 * Provide a method of storing and retrieving school-specific
 * information. Identifying schools is intended to be useful for
 * obtaining and storing preknowledge of the sections a school offers
 * to allow easier input.
 *
 * Anything code specific to a particular school should be placed in a
 * file in the school.d directory. The filename shall be the short,
 * alphanumeric, machine-usable school identifier followed by
 * ``.inc''. This allows optimized loading of school-specific routines
 * when the identifier is already known.
 */

/**
 * \brief
 *   Load a school profile based on its identifier.
 *
 * This function loads the school's description file and asks for info
 * from a callback called $school_id . '_info' which must return an
 * array with the following keys:
 * - name: a friendly name for the school. Must be a valid XHTML attribute string.
 * - url: the school's website URL as a valid XHTML attribute string. (i.e., escape ampersands).
 *
 * \param $school_id
 *   The school's alphanumeric identifier (which determines the name
 *   of the school's *.inc file).
 * \return
 *   A school_profile handle or NULL on error.
 */
function school_load($school_id)
{
  $school = array('id' => $school_id);

  /* guard against cracking attempts (protects against '../' and friends) */
  if (!preg_match('/^[0-9a-z]+$/', $school_id))
    return NULL;
  $school_file_name = dirname(__FILE__) . DIRECTORY_SEPARATOR
    . '..' . DIRECTORY_SEPARATOR . 'school.d' . DIRECTORY_SEPARATOR . $school_id . '.inc';

  if (!file_exists($school_file_name))
    return NULL;

  require_once($school_file_name);

  $school_info = $school_id . '_info';
  $school += $school_info();

  return $school;
}

/**
 * \brief
 *   Tries to guess what school a connection comes from.
 *
 * This function checks if $_REQUEST['school'] is set to a valid
 * school, so that the user can manually choose his school. Then it
 * chcecks if the user's session specifies what school profile to
 * use. Then it tries to make a best guess as to the school he's from
 * using the rDNS information provided by the httpd.
 *
 * \return
 *   A school profile or NULL if the school isn't in the session and
 *   can't be guessed.
 */
function school_load_guess()
{
  if (isset($_REQUEST['school']))
    {
      $school = school_load($_REQUEST['school']);
      if ($school)
	{
	  $_SESSION['school'] = $school['id'];
	  return $school;
	}
    }

  /* assume that we stored a valid school in the $_SESSION */
  if (isset($_SESSION['school']))
    return school_load($_SESSION['school']);

  if (isset($_SERVER['REMOTE_HOST']) || isset($_SERVER['REMOTE_ADDR']))
    {
      $addr = NULL;
      if (!isset($_SERVER['REMOTE_HOST']))
	$addr = gethostbyaddr($_SERVER['REMOTE_ADDR']);

      $cache = _school_cache_load();
      if ($addr && $cache && count($cache['domains']))
	{
	  $domain_parts = array_reverse(explode('.', $addr));
	  $domain_school = $cache['domains'];
	  while (is_array($domain_school))
	    {
	      $domain_part = array_shift($domain_parts);
	      if (isset($domain_school[$domain_part]))
		$domain_school = $domain_school[$domain_part];
	      else
		$domain_school = NULL;
	    }
	  /*
	   * by now, $domain_school is either NULL or the school_id of
	   * the school we want.
	   */
	  if ($domain_school)
	    {
	      $school = school_load($domain_school);
	      if ($school)
		{
		  $_SESSION['school'] = $domain_school;
		  return school_load($domain_school);
		}
	    }
	}
    }

  /*
   * set something in $_SESSION so that the gethostbyaddr() call
   * doesn't have to be done too often. (the isset() call above should
   * detect even the empty string).
   */
  $_SESSION['school'] = 'default';

  /* loading the school_id of 'default' MUST always work */
  return school_load($_SESSION['school']);
}

/**
 * \brief
 *   Render a list of school profile choices.
 *
 * Loads the list of schools and transforms the list into HTML,
 * optionally highlighting a specified school.
 *
 * The list of schools includes links to the specified destination,
 * appending a &school= to the query string. This is intended to work
 * in conjunction with school_load_guess() to allow the user to
 * manually choose his school.
 *
 * \param $highlight
 *   The school_id of the school whose list entry should be
 *   highlighted or NULL to avoid highlighting any entry.
 * \param $linkto
 *   Each school entry shall be a link for the user to switch which
 *   school profile he's using. This is to specify the URL or page
 *   these links should point to (the rest is handled by
 *   school_load_guess()). We will call htmlentities() for you.
 * \return
 *   An HTML formatted list of school profile choices where each entry
 *   is a link setting the client's choice to the specified school.
 */
function school_list_html($highlight = NULL, $linkto = NULL)
{
  $cache = _school_cache_load();
  if (!$cache || !count($cache['list']))
    return NULL;
  $school_list = $cache['list'];

  /* form the query string for the links */
  if (!$linkto)
    $linkto = '?';
  elseif (strpos($linkto, '?') === FALSE)
    $linkto .= '?';
  else
    $linkto .= '&';
  $linkto .= 'school=';
  $linkto = htmlentities($linkto);

  $html = "<ul>\n";
  foreach ($school_list as $school_id => $school_info)
    {
      $class_highlight = '';
      if ($school_id == $highlight)
	$class_highlight = ' highlight';
      $html .= '<li class="school' . $class_highlight . '"><a href="' . $linkto . $school_id . '">'
	. $school_info['name'] . "</a></li>\n";
    }
  $html .= "</ul>\n";

  return $html;
}

/**
 * \brief
 *   Get a school-specific information page.
 *
 * Each school may define a function called
 * <school_id>_instructions_html(). This is the wrapper which retrieves a
 * specific school's info HTML. It is recommended that instructions
 * about using the school's registration system in conjunction with
 * slate_permutate be placed in the instructions_html.
 *
 * \param $school
 *   A school handle obtained from school_load() or
 *   school_load_guess().
 * \return
 *   An HTML fragment of the school's information or NULL if the
 *   school either doesn't have any such information or if the school
 *   handle is invalid.
 */
function school_instructions_html($school)
{
  if (!$school || !$school['id']
      || !function_exists($school['id'] . '_instructions_html'))
    return NULL;

  $school_instructions_html = $school['id'] . '_instructions_html';
  return $school_instructions_html();
}

/**
 * \brief
 *   Used to load the school cache.
 *
 * \return
 *   The cache array or NULL if the cache couldn't be loaded.
 */
function _school_cache_load()
{
  static $cache = NULL;

  if ($cache != NULL)
    return $cache;

  $cache_file_name = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..'
    . DIRECTORY_SEPARATOR . 'cache' . DIRECTORY_SEPARATOR . 'schools';
  $cache_serialized = @file_get_contents($cache_file_name);
  if (isset($cache_serialized))
    $cache = unserialize($cache_serialized);

  return $cache;
}
