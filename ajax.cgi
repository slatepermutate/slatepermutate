#!/usr/bin/env php-cgi
<?php /* -*- mode: php; -*- */
/*
 * Copyright 2011 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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
 *   This file is an endpoint for generic AJAX requests (as opposed to
 *   autocompletion of course names).
 */

require_once('inc/school.inc');
require_once('inc/class.page.php');
require_once('inc/class.course.inc');

page::session_start();

/* should the following block of code be moved into a proposed Page::json()? */
if (isset($_REQUEST['txt'])) {
  header('Content-Type: text/plain; encoding=utf-8');
}
else {
  header('Content-Type: application/json; encoding=utf-8');
}

/**
 * \brief
 *   Wrap an error message up in JSON and send it.
 *
 * \param $message
 *   A valid XHTML fragment which will be wrapped in <div class="error" />
 */
function slate_permutate_json_error($message)
{
  echo json_encode(array('success' => FALSE, 'message' => '<div class="error">' . $message . '</div>'));
  exit;
}

/**
 * \brief
 *   Send a successful JSON response.
 *
 * \param $data
 *   A PHP array to be encoded with json_encode() and sent as
 *   obj.data.
 */
function slate_permutate_json_success(array $data = array())
{
  echo json_encode(array('success' => TRUE, 'data' => $data));
  exit;
}

if (isset($_REQUEST['school_registration_html']))
  {
    /*
     * Since we're just an AJAX callback, ask school_load_guess() not
     * to update $_SESSION with the school the user is using... And
     * make sure that the frontend actually tells us what school the
     * user is using ;-).
     */
    $school = school_load_guess(FALSE);
    if (empty($school))
      slate_permutate_json_error('Unable to load any school.');

    $page = page::page_create('');

    $courses = array();
    if (!empty($_REQUEST['courses']) && is_array($_REQUEST['courses']))
      {
	/*
	 * The below course deserialization blob should be moved into
	 * the Course object.
	 */
	foreach ($_REQUEST['courses'] as $course_json)
	  {
	    $course = Course::from_json_array($course_json);
	    if (!empty($course))
	      $courses[] = $course;
	  }
      }

    $html = school_registration_html($page, $school, $courses);
    if (empty($html))
      slate_permutate_json_error('School\'s registration information producer returned no data.');
    slate_permutate_json_success(is_array($html) ? $html : array('html' => $html));
  }

slate_permutate_json_error('Unrecognized command.');
