<?php /* -*- mode: php; -*- */
/*
 * Copyright 2011 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
 *
 * This file is part of SlatePermutate.
 *
 * SlatePermutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SlatePermutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with SlatePermutate.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once 'class.section.php';
require_once 'class.course_slot.inc';

/**
 * \brief
 *   Represents a Course containing multiple Sections.
 *
 * A course is associated with a certain subject material. For each
 * course, a student has to choose a particular Section to
 * take. Courses are not associated with professors or meeting times,
 * those are in the realm of the Section and SectionMeeting.
 *
 * Iterating over this object will return CourseSlot objects, which
 * act exactly like most universities' idea of a course. However, some
 * universities have one Course and require students to take one
 * section from each of different categories of sections within in a
 * course. For example, at umich for any course which has a listing of
 * Sections of the type 'discussion' the student _must_ take one of
 * these 'discussion' sections in addition to, for example, a
 * 'lecture' Section.
 */
class Course implements IteratorAggregate
{
  private $name;	// String
  private $title;       // String
  private $sections;	// Array of sections, for __wakeup() to convert to CourseSlots.
  private $nsections;	// int, for __wakeup() to convert to CourseSlots.
  /**
   * \brief
   *   Other courses that this course depends on.
   *
   * Example: Many calvin courses depend on lab courses.
   */
  private $dependencies;

  /**
   * \brief
   *    Creates a class with the given name.
   *
   * When updating this function, update the call to ``new Course()''
   * in Schedule::findPossibilities(), Schedule::writeoutTables(), and
   * Schedule::courses_get().
   *
   * \param $course_id
   *    The identifier of the class. Ex., MATH-101 in
   *    MATH-101-A. Retrieved with Course::getName().
   * \param $title
   *    The human-friendly course title, such as 'Introduction to
   *    Algebra', or NULL.
   */
  public function __construct($course_id, $title = NULL)
  {
    $this->slots = array();
    $this->name = $course_id;
    $this->title = $title;
    $this->dependencies = array();
  }

  /**
   * \brief
   *   Adds an already-instantiated Section to this class.
   *
   * \param $section
   *   The Section to append to this Course.
   * \param $course_slot_id
   *   The string identifer of the CourseSlot to place the given
   *   Section into. Most schools will not specify this.
   */
  public function section_add(Section $section, $course_slot_id = 'default')
  {
    if (empty($this->slots[$course_slot_id]))
      $this->slots[$course_slot_id] = new CourseSlot($course_slot_id);
    $this->slots[$course_slot_id]->section_add($section);
  }

  /**
   * \brief
   *   Append a CourseSlot to this Course.
   *
   * If this course already contains a CourseSlot with the same
   * CourseSlot identifier as $course_slot, then the new CourseSlot
   * will replace the old one.
   *
   * \param $course_slot
   *   The CourseSlot to append.
   */
  public function course_slot_add(CourseSlot $course_slot)
  {
    $this->slots[$course_slot->id_get()] = $course_slot;
  }

  /**
   * \brief
   *   Required function to implement the IteratorAggregate interface.
   */
  public function getIterator()
  {
    return new ArrayIterator($this->slots);
  }

  /**
   * \brief
   *   Retrieve a section of this Course based on its letter.
   *
   * This function will automatically search CourseSlots for this
   * Section. Why? Because even though we support multiple
   * CourseSlots, the section_id must _still_ be unique -- no two
   * CourseSlots should share a fully-qualified section_id.
   *
   * \return
   *   The requested section or NULL if that section does not yet
   *   exist for this class.
   */
  public function section_get($letter)
  {
    foreach ($this->slots as $slot)
      {
	$section = $slot->section_get($letter);
	if (!empty($section))
	  return $section;
      }
    return NULL;
  }

  /**
   * \brief
   *    Returns the name of the Course.
   * \return
   *    The name of the Course.
   */
  public function getName()
  {
    return $this->name;
  }

  /**
   * \brief
   *   Retrieve the human-friendly course title.
   *
   * \return
   *   A string, the human-friendly course title, or NULL if there is
   *   no title.
   */
  public function title_get()
  {
    return $this->title;
  }

  /**
   * \brief
   *   Add a dependency on another course.
   *
   * \param $course
   *   The other course to depend on.
   */
  public function dependency_add(Course $course)
  {
    $this->dependencies[] = $course;
  }

  /**
   * \brief
   *   Split up a user-friendly course specification into components.
   *
   * This will only return the 'department' and 'course' components of
   * the given course identifier. Otherwise, it acts the same as
   * Section::parse.
   *
   * \see Section::parse()
   *
   * \param $course_spec
   *   A course specifier to parse, such as 'cs262' or 'MATH-156'.
   * \return
   *   An array with normalized output having keys of 'department' and
   *   'course'. If the user's input has less than these two keys of
   *   information, the returned array may have zero or one elements.
   */
  public static function parse($course_spec)
  {
    $section_parts = Section::parse($course_spec);
    if (isset($section_parts['section']))
      unset($section_parts['section']);

    return $section_parts;
  }

  /**
   * \brief
   *   Represent this Course as a string.
   */
  public function __toString()
  {
    return $this->getName();
  }

  /**
   * \brief
   *   Represent this class as an array of sections ready to be JSONized.
   *
   * \param $recursion_trace
   *   Only for internal use. Used to prevent infinite recursion.
   */
  public function to_json_array(array $recursion_trace = array())
  {
    if (!empty($recursion_trace[$this->getName()]))
      return NULL;
    $recursion_trace[$this->getName()] = TRUE;

    $json_array = array(
			'course' => $this->getName(),
			'title' => $this->title_get(),
			'sections' => array(),
			'dependencies' => array(),
			);
    foreach ($this->slots as $slot)
      foreach ($slot->to_json_arrays() as $slot_json_section_array)
        $json_array['sections'][] = $slot_json_section_array;

    foreach ($this->dependencies as $dependency)
      {
	$dependency_json = $dependency->to_json_array($recursion_trace);
	if (!empty($dependency_json))
	  $json_array['dependencies'][] = $dependency_json;
      }

    return $json_array;
  }

  /**
   * \brief
   *   Produce a Course object based on a JSON array compatible with
   *   the output of Course::to_json_array().
   *
   * \param $json
   *   The JSON array to parse.
   * \return
   *   A Course.
   */
  public static function from_json_array($json)
  {
    $title = NULL;
    if (!empty($json['title']))
      $title = $json['title'];
    if (!empty($json['class']))
      {
	$json['course'] = $json['class'];
	unset($json['class']);
      }
    $course = new Course($json['course'], $title);

    if (!empty($json['sections']))
      {
	$json_course_slot_sections = array();
	foreach ($json['sections'] as $json_section)
	  {
	    $slot_id = 'default';
	    if (!empty($json_section['slot']))
	      $slot_id = $json_section['slot'];

	    if (empty($json_course_slot_sections[$slot_id]))
	      $json_course_slot_sections[$slot_id] = array();
	    $json_course_slot_sections[$slot_id][] = $json_section;
	  }

	foreach ($json_course_slot_sections as $slot_id => $json_course_slot_section)
	  $course->section_add(Section::from_json_arrays($json_course_slot_section), $slot_id);
      }

    if (!empty($json['dependencies']))
      foreach ($json['dependencies'] as $dependency)
	$course->dependency_add(Course::from_json_array($dependency));

    return $course;
  }

  /**
   * \brief
   *   Upgrade a course class to a newer version of that class.
   */
  public function __wakeup()
  {
    if (!isset($this->dependencies))
      $this->dependencies = array();

    if (!isset($this->title))
      $this->title = NULL;

    if (empty($this->slots) && !empty($this->sections))
      {
	$this->slots = array();

	foreach ($this->sections as $section)
	  $this->section_add($section);

	unset($this->sections);
	unset($this->nsections);
      }
  }
}
