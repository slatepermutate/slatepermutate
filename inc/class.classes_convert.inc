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

require_once('class.course.inc');

/**
 * \brief
 *   Class whose purpose is to upgrade deprecated saved_schedules in
 *   the Classes form into Course objects.
 */
class Classes
{
  private $name;	// String
  private $sections;	// Array of sections
  private $nsections;	// int

  public function to_course()
  {
    $course = new Course($this->name);

    foreach ($this->sections as $section)
      $course->section_add($section);

    return $course;
  }
}
