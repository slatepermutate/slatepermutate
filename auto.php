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

/**
 * \file
 *   This file's purpose is to autocomplete class names for supporting
 *   the autocomplete JS based off of crawling schools' registration
 *   websites. This shall only perform the autocompletion of class
 *   names.
 *
 *   Since we output JSON, no special Page classes and stuff
 *   :-p. Except we still call the Page class's session_start()
 *   function because we apparently need sessions.... oh yeah, for
 *   school profile supports ;-).
 */

require_once('inc/school.inc');
require_once('inc/class.page.php');
require_once('inc/class.course.inc');

page::session_start();

if (isset($_REQUEST['txt'])) {
  header('Content-Type: text/plain; encoding=utf-8');
}
else {
  header('Content-Type: application/json; encoding=utf-8');
}

if (!isset($_REQUEST['term'])) {
  clean_empty_exit();
}

$getsections = FALSE;
if (isset($_REQUEST['getsections'])) {
  $getsections = TRUE;
}

$term = $_REQUEST['term'];
$term_parts = Course::parse($term);
if (!count($term_parts)) {
  clean_empty_exit();
}

/*
 * We let the *_load_guess() functions check $_REQUEST['school'] and
 * $_REQUEST['semester'] for us, asking them not to update the
 * session.
 */
$school = school_load_guess(FALSE);
if (!$school['crawled']) {
  clean_empty_exit();
}
$semester = school_semester_guess($school, FALSE);

$cache_dir = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'cache' . DIRECTORY_SEPARATOR . 'auto'
  . DIRECTORY_SEPARATOR . $school['id'] . DIRECTORY_SEPARATOR . $semester['id'] . DIRECTORY_SEPARATOR;

/*
 * autocomplete the list of departments. If the user has already
 * entered a valid department name _and_ delimitted it, however, go on
 * to the next autocompletion step.
 */
$term_strlen = strlen($term);
$dept_strlen = strlen($term_parts['department']);
$dept = $term_parts['department'];
if (!$getsections && count($term_parts) == 1 && $term_strlen == strlen($dept))
  {
    $dept_file = $cache_dir . '-depts';
    if (!file_exists($dept_file)) {
      clean_empty_exit();
    }
    $departments = unserialize(file_get_contents($dept_file));
    $json_depts = array();
    if (!empty($departments) && is_array($departments[0]))
      {
	/* New format with department names/labels */
	foreach ($departments as $department)
	  if (!strncmp($department['value'], $dept, $dept_strlen))
	    $json_depts[] = $department;
      }
    else
      {
	/* Old format with just department id. */
	foreach ($departments as $department)
	  if (!strncmp($department, $dept, $dept_strlen))
	    $json_depts[] = $department;
      }

    echo json_encode($json_depts);
    exit(0);
  }

if ($getsections)
  {
    if (!isset($term_parts['course']))
      {
	/* user didn't give us enough information */
	header('HTTP/1.1 404: Nof found');
	header('Content-Type: text/plain; encoding=utf-8');
	echo 'Not a fully-qualified course name: ' . implode('-', $term_parts) . "\n";
	exit(0);
      }
    $section_file = $cache_dir . $dept . DIRECTORY_SEPARATOR . $term_parts['course'];
    if (file_exists($section_file))
      {
	readfile($section_file);
	exit(0);
      }
    /* Section not found! */
    header('HTTP/1.1 404: Not found');
    header('Content-Type: text/plain; encoding=utf-8');
    echo 'Could not find course ' . implode('-', $term_parts) . "\n";
    exit(0);
  }

/*
 * If a department is fully entered, life gets slightly more
 * complicated. I suppose I only want to autocomplete the first digit
 * of the course/class number. I.e., CS-2 for CS-262 for when the
 * student has entered CS- or 'CS'. But for now we can just dump the entire department at the user ;-).
 */
$classes_file = $cache_dir . $dept . '.sects';
if (file_exists($classes_file))
  {
    $classes = unserialize(file_get_contents($classes_file));
    $class_start = '';
    if (count($term_parts) > 1)
      $class_start = $term_parts['course'];

    /* reduce/create resultset */
    $json_classes = array();
    if (!empty($classes) && is_array($classes[0]))
      {
	$class_start = $dept . '-' . $class_start;
	$class_start_strlen = strlen($class_start);
	foreach ($classes as $course)
	  if (!strncmp($course['value'], $class_start, $class_start_strlen))
	    $json_classes[] = $course;
      }
    else
      {
	/* Old format with just course id. */
	$class_start_strlen = strlen($class_start);
	foreach ($classes as $class)
	  if (!strncmp($class, $class_start, $class_start_strlen))
	    $json_classes[] = $dept . '-' . $class;
      }

    echo json_encode($json_classes);
    exit(0);
  }

/**
 * Nothing caught
 */
clean_empty_exit();

/**
 * \brief
 *   Send an empty JSON array and exit.
 */
function clean_empty_exit()
{
  echo '[]';
  exit(0);
}
