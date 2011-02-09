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

$inc_dir = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;
require_once($inc_dir . 'class.course.inc');
require_once($inc_dir . 'class.section.php');

/**
 * \brief
 *   Identifies a school semester and acts as a container for courses
 *   offered in a semester.
 */
class Semester
{
  /**
   * \brief
   *   The Fall season.
   */
  const SEASON_FALL = 'fall';

  /**
   * \brief
   *   The Spring season.
   */
  const SEASON_SPRING = 'spring';

  /**
   * \brief
   *   The Summer season.
   */
  const SEASON_SUMMER = 'summer';

  /**
   * \brief
   *   Instantiate an empty Semester.
   *
   * \param $year
   *   The year of this semester. Must be four digits.
   * \param $season
   *   The season of this semester. Please use the constants
   *   Semester::SEASON_FALL, Semester::SEASON_SPRING, or
   *   Semester::SEASON_SUMMER if possible.
   * \param $time_start
   *   Specify a timestamp which roughly estimates when this semester
   *   starts to aid the algorithm for guessing the current
   *   semester. See Semester::time_start_set(), which may be used
   *   instead of this parameter
   * \param $time_end
   *   This may be specified now or via Semester::time_end_set().
   */
  function __construct($year, $season, $time_start = 0, $time_end = 0)
  {
    $this->time_start = 0;
    $this->time_end = 0;
    $this->season = $season;

    if (strlen($year) != 4 || !is_numeric($year))
      throw new ErrorException('Attempt to construct a Semester with an invalid year. The given year is `' . $year . '\'');
    $this->year = $year;

    $this->departments = array();
  }

  /**
   * \brief
   *   Add a class to this Semester.
   *
   * \param $class
   *   The class/course to add.
   */
  public function class_add(Course $class)
  {
    $class_parts = Course::parse($class->getName());
    if (!isset($class_parts['course']))
      throw new ErrorException('I was given a class with an invalid name: `' . $class->getName() . '\'');

    if (!isset($this->departments[$class_parts['department']]))
      $this->departments[$class_parts['department']] = array();
    $department =& $this->departments[$class_parts['department']];

    $department[$class_parts['course']] = $class;
  }

  /**
   * \brief
   *   Retrieve a class.
   *
   * \param $dept
   *   The class's department. 'CS' for 'CS-262'.
   * \param $class
   *   The course/class number. '262' for 'cs-262'.
   * \return
   *   A Course or NULL if not found.
   */
  public function class_get($dept, $class)
  {
    if (!isset($this->departments[$dept][$class]))
      return NULL;

    return $this->departments[$dept][$class];
  }

  /**
   * \brief
   *   Gets a list of departments available in this semester.
   */
  public function departments_get()
  {
    return array_keys($this->departments);
  }

  /**
   * \brief
   *   Gets a list of class/course numbers available for a particular
   *   department.
   */
  public function department_classes_get($dept)
  {
    if (!isset($this->departments[$dept]))
      throw new ErrorException('I was asked for a department I don\'t own: ' . $dept);

    return array_keys($this->departments[$dept]);
  }

  /**
   * \brief
   *   Utility function to add a section to the semester,
   *   automatically creating classes as necessary.
   *
   * \param $dept
   *   The department this section belongs to.
   * \param $class
   *   The class this section belongs to.
   * \param $section
   *   The section itself.
   */
  public function section_add($dept, $class, Section $section)
  {
    $dept = strtoupper($dept);
    $class = strtoupper($class);

    if (!isset($this->departments[$dept])
	|| !isset($this->departments[$dept][$class]))
      {
	$classobj = new Course($dept . '-' . $class);
	$this->class_add($classobj);
      }
    else
      {
	$classobj = $this->departments[$dept][$class];
      }

    $classobj->section_add($section);
  }

  /**
   * \brief
   *   Update the time_end.
   *
   * The time_end is a unix timestamp roughly estimating the time at
   * which a semester starts. It is used when guessing what semester a
   * user is interested in.
   *
   * \param $time_end
   *   The new time_end.
   */
  public function time_end_set($time_end)
  {
    $this->time_end = $time_end;
  }

  /**
   * \brief
   *   Set the time_end only if it would make the semester end later.
   *
   * Useful for crawler scripts incrementally guessing the endtime of
   * a semester.
   *
   * \param $time_end
   *   The new time_end to consider.
   */
  public function time_end_set_test($time_end)
  {
    if ($time_end && $time_end > $this->time_end)
      $this->time_end_set($time_end);
  }

  public function time_end_get()
  {
    return $this->time_end;
  }

  /**
   * \brief
   *   Update the time_start.
   *
   * The time_start is a unix timestamp roughly estimating the time at
   * which a semester starts. It is used when guessing what semester a
   * user is interested in.
   *
   * \param $time_start
   *   The new time_start.
   */
  public function time_start_set($time_start)
  {
    $this->time_start = $time_start;
  }

  /**
   * \brief
   *   Only update the time_start if the time_start isn't yet set or
   *   if the given time_start is earlier than the stored one.
   *
   * This should allow crawlers to easily accumulate proper time_start
   * and time_end values, see Semester::time_end_set_test();
   *
   * \param $time_start
   *   The new estimation of the semester's start.
   */
  public function time_start_set_test($time_start)
  {
    if ($time_start &&
	(!$this->time_start || $time_start < $this->time_start))
      $this->time_start_set($time_start);
  }

  public function time_start_get()
  {
    return $this->time_start;
  }

  /**
   * \brief
   *   Get a semester's year.
   */
  public function year_get()
  {
    return $this->year;
  }

  /**
   * \brief
   *   Get a semester's season.
   */
  public function season_get()
  {
    return $this->season;
  }

  /**
   * \brief
   *   Get a semester's friendly name:
   *
   * \return
   *   A string, the semester's friendly name.
   */
  public function name_get()
  {
    return ucfirst($this->season_get()) . ' ' . $this->year_get();
  }

  /**
   * \brief
   *   Return an identification string for this semester.
   *
   * Hopefully this identification string should be unique. Also, this
   * identification string is filesystem-safe.
   *
   * \return
   *   A string which may be used in paths or to uniquely identify
   *   this semester in the context of its school.
   */
  public function id()
  {
    return $this->year_get() . '_' . $this->season_get();
  }

  /**
   * \brief
   *   Enumerate all valid seasons.
   */
  public static function seasons_get_all()
  {
    return array(self::SEASON_SPRING, self::SEASON_SUMMER, self::SEASON_FALL);
  }
}
