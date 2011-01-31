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

//--------------------------------------------------
// class.section.php  Author:Nathan Gelderloos
//
// Represents a section of a class.
//--------------------------------------------------

require_once('inc/class.section_meeting.inc');
   
class Section
{

  private $letter;	// Section letter
  private $prof;	// Professor

  /* meeting times, array of SectionMeeting */
  private $meetings;

  /* the section synonym which uniquely identifies this section/course combination */
  private $synonym;

  /**
   * \brief
   *   Construct a Section.
   *
   * \param $letter
   *   The identifier (often a letter or numeral) of this section. For
   *   CS-262-A, this would be 'a'.
   * \param $section_meetings
   *   An array of SectionMeeting objects which describe all the
   *   different types of meetings this particular section has. It
   *   will be very uncommon for a course to have more than one such
   *   meeting time for a section. For example, Calvin doesn't have
   *   this. Another example, Cedarville lists different meeting times
   *   inside of a single section. Cedarville also lists all lectures
   *   and lab meeting times directly in a section's listing.
   * \param $synonym
   *   Some schools have a unique number for each section. This field
   *   is for that number.
   * \param $prof
   *   The faculty person(s) who teaches this section.
   */
  function __construct ($letter, array $section_meetings = array(), $synonym = NULL, $prof = NULL)
  {
    $this->letter = $letter;

    $this->meetings = $section_meetings;

    $this->synonym = $synonym;

    $this->prof = $prof;
  }

  public function getLetter()
  {
    return $this->letter;
  }

  public function getProf()
  {
    return $this->prof;
  }

  /**
   * \return
   *   This section's synonym -- a unique numeric identifier for this
   *   course. NULL if undefined.
   */
  public function getSynonym()
  {
    return $this->synonym;
  }

  /**
   * \brief
   *   Get an array of section meetings for this section.
   *
   * \return
   *   An array of SectionMeeting objects.
   */
  public function getMeetings()
  {
    return $this->meetings;
  }

  /**
   * \brief
   *   Check if this section conflicts with the given section.
   *
   * \param $that
   *   The other section for which I should check for conflicts.
   * \return
   *   TRUE if there is a conflict, FALSE otherwise.
   */
  public function conflictsWith(Section $that)
  {
    foreach ($this->meetings as $this_meeting)
      foreach ($that->meetings as $that_meeting)
      if ($this_meeting->conflictsWith($that_meeting))
	return TRUE;

    return FALSE;
  }

  /**
   * \brief
   *   Add another section meeting time to this section.
   *
   * Useful for process.php when it's calling
   * Schedule::addSectionMeeting() multiple times.
   */
  public function meeting_add(SectionMeeting $meeting)
  {
    $this->meetings[] = $meeting;
  }
  
  /**
   * \brief
   *   Splits up a section specifier into dept, course number, and
   *   section.
   *
   * For example, will return array('CS', '262', 'A') for 'CS-262-A'
   * or 'CS262 A' or 'cs-262,a'. This function is not for dealing with
   * course synonyms.
   *
   * Note: Section specifiers where the section numeral/letter is
   * directly adjacent to the course number is not valid. Calvin
   * College distinguishes between normal courses and their labs by
   * appending an `L' to the course number. Thus, 'CS262A' is not a
   * valid specifier for 'CS-262-A' because there may exist another
   * course called 'CS-262L-A' (which is likely the lab for the
   * 'CS-262-A' class ;-)).
   *
   * \param $section_spec
   *   A string starting with a section specifier. If only the
   *   department is found, an array of size one is returned. If the
   *   course number is also found, both department and course id are
   *   returned. If all three are found, the array has three elements.
   *
   *   This array is keyed, so the found items may be referred to as
   *   'deptartment', 'course', and 'section'.
   *
   * \return
   *   An array with the department, course number, and section
   *   identifier. This array may be empty or have from one through
   *   three elements depending on the validity and precision of the
   *   $section_spec.
   */
  public static function parse($section_spec)
  {
    $ret = array();

    $section_spec = trim($section_spec);
    if (!preg_match(';([a-zA-Z]+)[^0-9]*;', $section_spec, $dept_matches))
      return $ret;

    /*
     * remove away the already-parsed stuff, including gunk between the
     * dept and the course num.
     */
    $section_spec = trim(substr($section_spec, strlen($dept_matches[0])));
    $ret['department'] = strtoupper($dept_matches[1]);

    if (!preg_match(';([0-9a-zA-Z]+)[^a-zA-Z0-9]*;', $section_spec, $course_matches))
      return $ret;

    /* skip gunk */
    $section_spec = trim(substr($section_spec, strlen($course_matches[0])));
    $ret['course'] = strtoupper($course_matches[1]);

    /*
     * we accept _either_ alphabetic section _or_ numeric section (the
     * latter is for cedarville, particulaly)
     */
    if (!preg_match(';([0-9]+|[a-zA-Z]+);', $section_spec, $section_matches))
      return $ret;

    $ret['section'] = strtoupper($section_matches[1]);

    return $ret;
  }

  /**
   * \brief
   *   Get arrays of information needed by the AJAX stuff.
   *
   * \return
   *   An array of arrays that should be merged with the return value
   *   of other Section::to_json_array() calls.
   */
  public function to_json_arrays()
  {
    $json_arrays = array();

    foreach ($this->meetings as $meeting)
      {
	$json_array = array('section' => $this->letter,
			    'prof' => $this->prof,
			    'synonym' => $this->synonym,
			    );

	$json_array += $meeting->to_json_array();
	$json_arrays[] = $json_array;
      }

    return $json_arrays;
  }


  /* for legacy unserialization */
  private $start;
  private $tend;
  private $bdays;

  /**
   * \brief
   *   A magic function which tries to upgrade old serialized sections
   *   to the new format.
   */
  public function __wakeup()
  {
    /* upgrade to SectionMeeting stuffage */
    if (!empty($this->start))
      {
	$days = '';
	$daymap = array(0 => 'm', 1 => 't', 2 => 'w', 3 => 'h', 4 => 'f');
	foreach ($this->bdays as $day => $have_day)
	  if ($have_day)
	    $days .= $daymap[$day];

	/* the old format had a ->prof but initialied it to ``unknown prof'' */
	$this->prof = '';

	$this->meetings = array();
	$this->meeting_add(new SectionMeeting($days, $this->start, $this->tend, '', 'lecture'));

	/*
	 * if we're reserialized in the future, make sure we don't do this same upgrade procedure again ;-).
	 */
	unset($this->start);
      }
  }
}