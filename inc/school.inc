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
 * - example_course_id: An example course identifier representative of a school's course IDs. (e.g., CS-101 for Calvin).
 * - id: The school's ID.
 *
 * \param $school_id
 *   The school's alphanumeric identifier (which determines the name
 *   of the school's *.inc file).
 * \param $load_all_inc
 *   Asks for a school's extraneous .inc files to be loaded
 *   to. Intended for use by rehash.php only.
 * \return
 *   A school_profile handle or NULL on error.
 */
function school_load($school_id, $load_all_inc = FALSE)
{
  $school = array('id' => $school_id);

  /* guard against cracking attempts (protects against '../' and friends) */
  if (!preg_match('/^[0-9a-z]+$/', $school_id))
    return NULL;
  $school_file_name_base = dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'school.d' . DIRECTORY_SEPARATOR;
  $school_file_name = $school_file_name_base . $school_id . '.inc';

  if (!file_exists($school_file_name))
    return NULL;

  require_once($school_file_name);
  if ($load_all_inc)
    {
      $school_crawl_file_name = $school_file_name_base . $school_id . '.crawl.inc';
      if (file_exists($school_crawl_file_name))
	require_once($school_crawl_file_name);
    }

  $school_info = $school_id . '_info';
  $school += $school_info();

  /*
   * append small amount of info from the cache entry for this school:
   * whether or not it was crawled.
   *
   * Perhaps this stuff should be just moved into the _info function
   * for efficiency.
   */
  $cache = _school_cache_load();
  if ($cache && count($cache['list']) && isset($cache['list'][$school['id']]))
    {
      $school['crawled'] = $cache['list'][$school['id']]['crawled'];

      $school_semesters_filename = dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'cache'
	. DIRECTORY_SEPARATOR . 'auto' . DIRECTORY_SEPARATOR . $school['id']
	. DIRECTORY_SEPARATOR . '-semesters';
      if (file_exists($school_semesters_filename))
	$school['semesters'] = unserialize(file_get_contents($school_semesters_filename));
      else
	$school['semesters'] = array();
    }

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
 * \param $update_session
 *   Whether or not the results should be stored into the session for
 *   later use. A value of disabled makes sense for the auto.php AJAX
 *   callback script, where the user is not loading a page himself but
 *   the current school is being specified in the URL
 *   parameters... thus updating the session value here would be a
 *   side-effect. We're doing this so that the user can do
 *   autocomplete for two different school/semester pairs in two
 *   different browser tabs under the same session.
 *
 * \return
 *   A school profile or NULL if the school isn't in the session and
 *   can't be guessed.
 */
function school_load_guess($update_session = TRUE)
{
  if (isset($_REQUEST['school']))
    {
      $school = school_load($_REQUEST['school']);
      if ($school)
	{
	  if ($update_session)
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
		  if ($update_sesssion)
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
  if ($update_session)
    $_SESSION['school'] = 'default';

  /* loading the school_id of 'default' MUST always work */
  return school_load('default');
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
  global $school_default_school;

  if (empty($school) || empty($school['id']))
    /*
     * Invalid param deserves a NULL :-p. Really, this invalid param
     * handling shouldn't be needed...
     */
    return NULL;

  $school_instructions_html = $school['id'] . '_instructions_html';

  if (!function_exists($school_instructions_html))
    {
      /* load the default school's _instructions_html() function */
      if ($school_default_school === NULL)
	$school_default_school = school_load('default');
      /* ``hacky'', but preferable to recursion: */
      $school_instructions_html = 'default' . '_instructions_html';

      /* be 503-safe */
      if (!function_exists($school_instructions_html))
	return NULL;
    }

  return $school_instructions_html();
}

/**
 * \brief
 *   Get CSS specific to a school.
 *
 * For a school to get custom CSS into slate_permutate's <head />,
 * just create a <school_id>_page_css($school) function which returns
 * a string of CSS.
 *
 * \param $school
 *   The school from which to fetch CSS.
 * \return
 *   A string of valid CSS.
 */
function school_page_css(array $school)
{
  if (empty($school))
    return '';
  $school_page_css = $school['id'] . '_page_css';
  if (!function_exists($school_page_css))
    return '';
  return $school_page_css($school);
}

/**
 * \brief
 *   Return information about available semesters.
 *
 * \param $school
 *   The school.
 * \return
 *   An array with keys being semester IDs ordered by weights with
 *   lowest first and keys of 'id' (the semester's ID), 'name' (the
 *   friendly name), and 'weight' (lower numbers mean these semesters
 *   should be earlier, may be positive or negative). 'time_start',
 *   'time_end' are unix timestamps estimating the begin and end point
 *   of each semester.
 */
function school_semesters(array $school)
{
  if (empty($school['crawled']))
    return array();
  return $school['semesters'];
}

/**
 * \brief
 *   Return the semester which either the user has selected or which
 *   makes the most sense.
 *
 * \param $school
 *   The school for which a semester should be guessed.
 * \param $update_session
 *   Whether or not $_SESSION should be updatd with the new value. A
 *   value of FALSE makes sense for the ajax.php callback script.
 * \return
 *   An array with the keys 'id', 'name', and 'weight' corresponding
 *   to the same keys in the arrays returned by school_semesters() or
 *   NULL if no semester can be found.
 */
function school_semester_guess(array $school, $update_session = FALSE)
{
  $semesters = school_semesters($school);

  if (!empty($_REQUEST['semester'])
      && isset($semesters[$_REQUEST['semester']]))
    {
      $semester = $semesters[$_REQUEST['semester']];
      if ($update_session)
	$_SESSION['semester'] = $semester['id'];
      return $semester;
    }

  if (!empty($_SESSION['semester'])
      && isset($semesters[$_SESSION['semester']]))
      return $semesters[$_SESSION['semester']];

  /*
   * The following is the most _common_ scenario:
   *
   * A student is looking ahead in the last half of March (3) to
   * register for a semester starting in September (9) and ending in
   * December. Thus, looking 6 months into the future may put us right
   * in the middle of the desired semester, also considering that
   * during the summer (6) one is looking to register for a fall
   * semester which ends in December (12).
   */
  $time_target = time() + 60*60*24*365.25 * 0.5;

  $semester = NULL;
  /* guessed semester */
  $best_semester = NULL;
  /*
   * The absolute value of the difference between the $time_target and
   * the middle of the guessed semester. Smaller is better.
   */
  $best_score = -1;
  foreach ($semesters as $semester)
    {
      $my_score = abs(($semester['time_end'] + $semester['time_start']) / 2 - $time_target);
      if ($best_score == -1 || $my_score < $best_score)
	{
	  $best_semester = $semester;
	  $best_score = $my_score;
	}
    }
  if (!empty($best_semester))
    return $best_semester;
  return $semester;
}

/**
 * \brief
 *   Return an array of default classes for a particular school.
 *
 * \param $school
 *   The school's handle.
 */
function school_default_courses($school)
{
  $school_default_courses = $school['id'] . '_default_courses';
  if (function_exists($school_default_courses))
    {
      require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'class.course.inc');
      return $school_default_courses();
    }

  return array();
}

/**
 * \brief
 *   Return an example course id for the school.
 *
 * Each school may specify an example course ID by placing a key
 * called 'example_course_id' into the array returned by its
 * <school_id>_info() function. See school_load().
 *
 * \param $school
 *   The school's handle.
 * \return
 *   A string containing a representative example of a course ID for
 *   the given school.
 */
function school_example_course_id(array $school)
{
  return $school['example_course_id'];
}

/**
 * \brief
 *   Populate a ``Registration Codes'' dialog.
 *
 * A school may override the default output by writing a function with
 * the same signature and semantics as this function with a name of
 * <school_id>_registration_html().
 *
 * \param $page
 *   The page object; used to conditionally format code as HTML or
 *   XHTML. Remember, you are writing an XHTML fragment and should not
 *   call Page::foot() or Page::head().
 * \param $school
 *   The school handle.
 * \param $courses
 *   An array of courses, where each course only has one section which
 *   is the section which the user chose.
 * \return
 *   A string which is a valid XHTML fragment. This fragment should
 *   direct the user to his school's registration services. It should
 *   also render the list of sections in a way appropriate to that
 *   school -- such as a list of fully-qualified section_ids or a
 *   listing of section synonyms.
 */
function school_registration_html(Page $page, array $school, array $courses)
{
  /*
   * The school from which to call the <school_id>_registration_html()
   * function. Used to fall back onto the 'default' school if the
   * selected school doesn't have a <school_id>_registration_html().
   */
  $function_school = $school;
  if (!function_exists($function_school['id'] . '_registration_html'))
    {
      $function_school = school_load('default');
      if (!function_exists($function_school['id'] . '_registration_Html'))
	return '<div class="error">Unable to load generic <tt>school_registration_html()</tt> function.</div>';
    }
  $school_registration_html = $function_school['id'] . '_registration_html';
  return $school_registration_html($page, $school, $courses);
}

/**
 * \brief
 *   Determine if a school has crawler data stored.
 *
 * \param $school
 *   The which should be checked.
 */
function school_has_auto(array $school)
{
  return isset($school['crawled']) && $school['crawled'];
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
  if (!empty($cache_serialized))
    $cache = unserialize($cache_serialized);

  return $cache;
}
