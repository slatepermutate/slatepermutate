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

require_once dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR . 'school.crawl.webadvisor.inc';

/**
 * \brief
 *   Retrieve a list of crawlable semesters from Calvin College.
 *
 * \param $school
 *   The calvin school handle.
 * \param $semesters
 *   The array to populate with empty Semester objects.
 * \param $school_crawl_log
 *   A school_crawl_log handle for informing the user/developer of
 *   progress.
 */
function calvin_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  return school_crawl_webadvisor_semester_list($school, $semesters, $school_crawl_log);
}

/**
 * \brief
 *   Crawl the courses for a semester from Calvin College.
 *
 * \param $school
 *   The calvin school handle.
 * \param $semester
 *   The Semester object to populate with courses.
 * \param $school_crawl_log
 *   The logger handle.
 */
function calvin_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  return school_crawl_webadvisor_semester($school, $semester, $school_crawl_log);
}

/**
 * \brief
 *   Add a course to a semester if that semester doesn't yet have this
 *   course.
 *
 * \param $semester
 *   The semester to which the course should be appended.
 * \param $deparmtent
 *   The department of the course to add.
 * \param $course_id
 *   The course_id which, with the department string, forms a
 *   fully-qualified course_id.
 */
function calvin_crawl_course_add_(Semester $semester, $department, $course_id, $title)
{
  if ($semester->class_get($department, $course_id) == NULL)
    $semester->class_add(new Course($department . '-' . $course_id, $title));
}
