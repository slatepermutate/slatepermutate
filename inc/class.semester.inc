<?php /* -*- mode: php; indent-tabs-mode: nil; -*- */
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
require_once($inc_dir . 'math.inc');

/**
 * \brief
 *   Identifies a school semester and acts as a container for Course s
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
    $this->time_starts = array();
    $this->time_ends = array();
    $this->season = $season;

    if (strlen($year) != 4 || !is_numeric($year))
      throw new ErrorException('Attempt to construct a Semester with an invalid year. The given year is `' . $year . '\'');
    $this->year = $year;

    $this->departments = array();
    $this->department_names = array();

    /*
     * For $this->popular_course_get().
     */
    $this->course_num_sections_map = array();
  }

  /**
   * \brief
   *   Add a class to this Semester.
   *
   * \param $class
   *   The class/course to add.
   */
  public function class_add(Course $course)
  {
    $course_parts = Course::parse($course->getName());
    if (!isset($course_parts['course']))
      throw new ErrorException('I was given a class with an invalid name: `' . $course->getName() . '\'');

    foreach ($course as $course_slot)
      foreach ($course_slot as $section)
      {
        foreach ($section as $meeting)
          $this->time_set_section_meeting($meeting);

        $this->_section_count($course, $section);
      }

    if (!isset($this->departments[$course_parts['department']]))
      $this->departments[$course_parts['department']] = array();
    $department =& $this->departments[$course_parts['department']];

    $department[$course_parts['course']] = $course;
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
   *   Set a department's human-friendly name.
   *
   * \param $dept
   *   The department.
   * \param $name
   *   The human-friendly name of the department.
   */
  public function department_name_set($dept, $name)
  {
    $this->department_names[$dept] = $name;
  }

  /**
   * \brief
   *   Determine if a particular department has a human-oriented name.
   *
   * \param $dept
   *   The department to check.
   * \return
   *   TRUE if this department has been assigned a human-friendly
   *   name.
   */
  public function department_name_has($dept)
  {
    return !empty($this->department_names[$dept]);
  }

  /**
   * \brief
   *   Get a list of departments and long names, if available.
   *
   * This function only returns departments for which this Semester
   * has courses.
   *
   * \return
   *   An array where keys are department identifiers and values are
   *   human-friendly names (if available). If human-friendly names
   *   are not available, department identifiers are used instead of
   *   the human-friendly ones.
   */
  public function department_names_get()
  {
    $department_names = array();
    foreach ($this->departments_get() as $dept)
      if (empty($this->department_names[$dept]))
        $department_names[$dept] = $dept;
      else
        $department_names[$dept] = $this->department_names[$dept];
    return $department_names;
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
   * Crawler functions should generally use this instead of
   * Semester::class_add().
   *
   * \param $dept
   *   The department this section belongs to.
   * \param $class
   *   The class this section belongs to.
   * \param $section
   *   The section itself.
   * \param $title
   *   The course human-friendly title.
   * \param $course_slot_id
   *   The slot of the course which this section should be added
   *   to. Use 'default' (or don't pass this parameter) if your school
   *   does not have the concept of course slots. Ask binki for help
   *   figuring this out. Course slots are a sort of
   *   inverse/complement to section_meetings.
   */
  public function section_add($dept, $class, Section $section, $title = NULL, $course_slot_id = 'default') {
    foreach ($section as $meeting) {
      $this->time_set_section_meeting($meeting);
    }

    $dept = strtoupper($dept);
    $class = strtoupper($class);

    if (!isset($this->departments[$dept])
        || !isset($this->departments[$dept][$class])) {
      $classobj = new Course($dept . '-' . $class, $title);
      $this->class_add($classobj);
    } else {
      $classobj = $this->departments[$dept][$class];
    }

    $classobj->section_add($section, $course_slot_id);

    $this->_section_count($classobj, $section);
  }

  /**
   * \brief
   *   Add a section_meeting, calling Semester::section_add() as
   *   necessary.
   *
   * To be used by crawlers when parsing data which only presents one
   * section_meeting at a time. I.e., when they do tabular data right.
   *
   * \param $dept
   *   The department this section_meeting's course belongs to.
   * \param $course
   *   The course number this section_meeting's section belongs to.
   * \param $title
   *   The course title of the given course the section_meeting or
   *   NULL. If the section has already been added, this parameter
   *   will be ignored.
   * \param $section
   *   The letter or numbers which make up the section's name.
   * \param $synonym
   *   The section synonym or NULL.
   * \param $section_meeting
   *   The SectionMeeting to be added to a section which may or may
   *   not already be in this Semester.
   * \param $course_slot_id
   *   The name of the new CourseSlot to create if the given section
   *   does not yet exist.
   * \param $credit_hours
   *   The number of credit hours of the associated course or a
   *   negative value if unknown.
   */
  public function section_meeting_add($dept, $course, $title, $section, $synonym, SectionMeeting $section_meeting, $course_slot_id = 'default', $credit_hours = -1.0, $sectionTitle = NULL)
  {
    $this->time_set_section_meeting($section_meeting);

    $dept = strtoupper($dept);
    $course = strtoupper($course);

    if (empty($this->departments[$dept][$course])) {
      $course_obj = NULL;
    } else {
      $course_obj = $this->departments[$dept][$course];
      $section_obj = $course_obj->section_get($section);
    }
    if (empty($course_obj) || empty($section_obj)) {
      return $this->section_add($dept, $course, new Section($section, array($section_meeting), $synonym, $credit_hours, $sectionTitle), $title, $course_slot_id, $sectionTitle);
    }

    $section_obj->meeting_add($section_meeting);
    return;
  }

  /**
   * \brief
   *   Account for the addition of a new section to this Semester.
   *
   * \param $course
   *   The course this section is a part of.
   * \param $section
   *   The section.
   */
  private function _section_count(Course $course, Section $section)
  {
    $fully_qualified_course_id = $course->getName();
    $this->course_num_sections_map += array($fully_qualified_course_id => 0);
    $this->course_num_sections_map[$fully_qualified_course_id] ++;
  }

  /**
   * \brief
   *   Get the most popular course.
   *
   * \return
   *   The fully-qualified course_id of the most popular course in
   *   this Semester.
   */
  public function popular_course_id_get()
  {
    if (count($this->course_num_sections_map))
      {
        arsort($this->course_num_sections_map, SORT_NUMERIC);
        reset($this->course_num_sections_map);
        $this->_popular_course = key($this->course_num_sections_map);
      }
    if (!isset($this->_popular_course))
      {
        /* The default popular course */
        $this->_popular_course = 'ENGL-101';
      }
    return $this->_popular_course;
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

  /**
   * \brief
   *   Add a potential end time to the pool of end times.
   */
  public function time_end_pool_add($time_end)
  {
    $this->time_ends[] = $time_end;
  }

  public function time_end_get()
  {
    if (count($this->time_ends))
      {
        $times = filter_outliers($this->time_ends);
        $this->time_end = max($times);
      }
    elseif (!$this->time_end)
      {
        /* Return, but don't store, a guess. */
        $fourmonths = gmmktime(0, 0, 0, 5, 2012) - gmmktime(0, 0, 0, 1, 2012);
        return $this->time_start_get() + $fourmonths;
      }

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

  /**
   * \brief
   *   Add a potential semester start time to the pool of potential
   *   start times.
   *
   * The idea is that there might be erroneous entries in a school's
   * database (
   * http://www.facebook.com/CalvinRegistrar/posts/299438720070457 )
   * which would skew the detected start time. Use statistics to
   * detect and kill outliers by using a pool of endtimes :-D.
   */
  public function time_start_pool_add($time_start)
  {
    $this->time_starts[] = $time_start;
  }

  public function time_start_get()
  {
    if (count($this->time_starts)) {
      $times = filter_outliers($this->time_starts);
      $this->time_end = min($times);
    } elseif (!$this->time_start) {
      /* Return, but don't store, a guess. */
      $season_starts = array(
        'spring' => 0,
        'summer' => 5,
        'fall' => 8,
        'winter' => 12,
      );
      if (!empty($season_starts[$this->season])) {
        return gmmktime(0, 0, 0, $season_starts[$this->season], 1, $this->year);
      }
    }

    return $this->time_start;
  }

  /**
   * \brief
   *   Consider a section_meeting's start and end dates and make
   *   appropriate time_start_set_test() and time_end_set_test()
   *   calls.
   */
  public function time_set_section_meeting(SectionMeeting $meeting)
  {
    $date_start = $meeting->date_start_get();
    if (!empty($date_start))
      $this->time_start_set_test($date_start);

    $date_end = $meeting->date_end_get();
    if (!empty($date_end))
      $this->time_end_set_test($date_end);
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
   *   Handle conversion to a string.
   *
   * \return
   *   A string.
   */
  public function __tostring()
  {
    return $this->name_get();
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

  /**
   * \brief
   *   Clean the semester of all sections, keeping metadata intact.
   */
  public function purge()
  {
    $this->departments = array();
    /*
     * Make sure that time_end is set to the proper end time before
     * clearing out the pool in the time_ends array.
     */
    $this->time_end_get();
    $this->time_ends = array();
    $this->time_start_get();
    $this->time_starts = array();

    /*
     * A mapping which keeps track of how many sections any given
     * course has, with course_id '-' section_id as the key and the
     * count as the value. Used to calculate the most frequently-used
     * course_id to use as the example course_id (bug #102).
     */
    $this->popular_course_id_get();
    $this->course_num_sections_map = array();
  }
}
