<?php
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

/**
 * \brief
 *   A package of Section objects of which one must be taken to sign
 *   up for a particular Course.
 *
 * For example, some schools like umich have a single Course where one
 * must sign up for one Section for every meeting_type of the
 * following: 'lecture', 'discussion', and 'lab'. This way they avoid
 * creating separate Course objects for labs (which is calvin's
 * solution to the problem).
 *
 * Many schools do not have the CourseSlot paradigm. These will work
 * just fine using one default CourseSlot.
 *
 * Iterating over this object will yield Section objects.
 */
class CourseSlot implements IteratorAggregate
{
  /**
   * \brief
   *   An array of Section objects associated with this CourseSlot.
   */
  private $sections;

  /**
   * \brief
   *   An identifier for this slot. Used during crawling when sorting
   *   Sections into CourseSlot objects.
   */
  private $id;

  /**
   * \brief
   *   Creates a CourseSlot with the given identifier.
   */
  public function __construct($id)
  {
    $this->id = $id;
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
   *   Get the identifier of this slot.
   *
   * \return
   *   The slot's identifier string.
   */
  public function id_get()
  {
    return $this->id;
  }

  /**
   * \brief
   *   Appends a Section to this CourseSlot.
   *
   * \param $section
   *   The Section to append.
   */
  public function section_add(Section $section)
  {
    /*
     * This behavior of the Schedule class requires this manner of
     * indexing sections because it iterates using for ($count = 0;
     * $count < ...) -style loops. Thus we allow PHP's natural
     * indexing mechanism to do its job...
     */
    $this->sections[] = $section;
  }

  /**
   * \brief
   *    Returns the number of sections in the class.
   */
  function sections_count()
  {
    return count($this->sections);
  }

  /**
   * \brief
   *    Returns the desired section for analysis.
   * \return
   *    The selected section of the course.
   */
  function section_get_i($i)
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
   *   Get the JSON arrays of data specific to each Section, adding
   *   slight metadata for this SLOT.
   *
   * There is no corresponding from_json_arrays() function for this
   * class. See Course::from_json_array() which manages the conversion
   * of JSON slots to CourseSlot objects.
   */
  public function to_json_arrays()
  {
    $slot_section_json_arrays = array();
    foreach ($this->sections as $section)
      {
	$section_json_arrays = $section->to_json_arrays();
	foreach ($section_json_arrays as $section_json_array)
	  $slot_section_json_arrays[] = $section_json_array + array('slot' => $this->id);
      }
    return $slot_section_json_arrays;
  }
}
