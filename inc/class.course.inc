<?php
/*
 * Copyright 2010 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
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

/**
 * \file
 *    This file represents a course (formerly class). It stores
 *    the section associated with the course.
 */

include_once 'class.section.php';

class Course implements IteratorAggregate
{
  private $name;	// String
  private $sections;	// Array of sections
  private $nsections;	// int
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
   * \param $n
   *    The name of the class.
   */
  function __construct($n)
  {
    $this->sections = array();
    $this->name = $n;
    $this->nsections = 0;
    $this->dependencies = array();
  }
	
  /**
   * \brief
   *   Adds an already-instantiated section to this class.
   */
  public function section_add(Section $section)
  {
    $this->sections[$this->nsections] = $section;
    $this->nsections++;
  }

  /**
   * \brief
   *   Required function to implement the IteratorAggregate interface.
   */
  public function getIterator()
  {
    return new ArrayIterator($this->sections);
  }

  /**
   * \brief
   *    Returns the number of sections in the class.
   */
  function getnsections()
  {
    return $this->nsections;
  }
	
  /**
   * \brief
   *    Returns the desired section for analysis.
   * \return
   *    The selected section of the course.
   */
  function getSection($i)
  {
    $result = $this->sections[$i];
    return $result;
  }

  /**
   * \brief
   *   Retrieve a section of this class based on its letter.
   *
   * \todo Make this function replace completely the getSection()
   * function, have $this->sections be keyed by letter, and have a
   * __wakup() convert the old $this->sections format to the new one.
   *
   * \return
   *   The requested section or NULL if that section does not yet
   *   exist for this class.
   */
  public function section_get($letter)
  {
    foreach ($this->sections as $section) {
      if ($section->getLetter() == $letter) {
	return $section;
      }
    }
    return NULL;
  }

  /**
   * \brief
   *    Returns the name of the class.
   * \return
   *    The name of the class.
   */
  public function getName()
  {
    return $this->name;
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
    if (isset($section_parts['section'])) {
      unset($section_parts['section']);
    }

    return $section_parts;
  }

  /**
   * \brief
   *   Represent this class as a string.
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
			'class' => $this->getName(),
			'sections' => array(),
			'dependencies' => array(),
			);
    foreach ($this->sections as $section)
      {
	$section_json_arrays = $section->to_json_arrays();
	foreach ($section_json_arrays as $section_json_array)
	  $json_array['sections'][] = $section_json_array;
      }

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
    $course = new Course($json['class']);

    if (!empty($json['sections']))
      $course->section_add(Section::from_json_arrays($json['sections']));
    
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
  }
}
