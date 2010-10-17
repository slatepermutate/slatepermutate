<?php

//--------------------------------------------------
// class.section.php  Author:Nathan Gelderloos
//
// Represents a section of a class.
//--------------------------------------------------
   
class Section
{

  private $letter;	// Section letter
  private $prof;	// Professor
  private $start;	// Start time
  private $tend;	// End time
  private $idays;	// Integer version of meeting days
  private $bdays;	// Boolean array of meeting days

  /**
   * \brief
   *   Construct a Section.
   *
   * \param $letter
   *   The identifier (often a letter or numeral) of this section. For
   *   CS-262-A, this would be 'a'.
   * \param $time_start
   *   The time of day when this section meets. Formatted as a string,
   *   with the 24-hr representation of the hour taking the first two
   *   characters and a two-digit representation of minutes taking the
   *   next two characters.
   * \param $time_end
   *   The time of day when this section's meeting is over.
   * \param $days
   *   A string representing the days that this section meets. The
   *   format of this string is an ordered series of numerals less
   *   than or equal to 5. Each numeral from 1 through 5 represents
   *   one of Monday, Tuesday, Wednesday, Thursday, and Friday. For
   *   example, '135' would be for a course which meets on Monday,
   *   Wednesday, and Friday.
   * \param $prof
   *   The faculty person(s) who teaches this section.
   * \param $room
   *   An identifier of the room within which the section is taught.
   */
  function __construct ($letter, $time_start, $time_end, $days,
			$synonym = NULL, $prof = NULL, $room = NULL)
  {
    $this->letter = $letter;
    $this->start = $time_start;
    $this->tend = $time_end;

    $this->idays = $days;
    $this->bdays = $this->setbdays();

    $this->synonym = $synonym;
    $this->prof = $prof;
    $this->room = $room;
  }

  private function setbdays()
  {
    $result = array(FALSE, FALSE, FALSE, FALSE, FALSE);

    if($this->idays == 12345)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = true;}

    if($this->idays == 1234)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = false;}
    if($this->idays == 1235)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = true;}
    if($this->idays == 1245)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = true;}
    if($this->idays == 1345)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = true;}
    if($this->idays == 2345)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = true;}

    if($this->idays == 123)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = false;}
    if($this->idays == 124)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = false;}
    if($this->idays == 125)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = true;}
    if($this->idays == 134)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = false;}
    if($this->idays == 135)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = true;}
    if($this->idays == 145)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = true;}
    if($this->idays == 234)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = false;}
    if($this->idays == 235)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = true;}
    if($this->idays == 245)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = true;}
    if($this->idays == 345)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = true;}

    if($this->idays == 12)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = false;}
    if($this->idays == 13)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = false;}
    if($this->idays == 14)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = false;}
    if($this->idays == 15)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = true;}
    if($this->idays == 23)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = false;}
    if($this->idays == 24)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = false;}
    if($this->idays == 25)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = true;}
    if($this->idays == 34)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = false;}
    if($this->idays == 35)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = true;}
    if($this->idays == 45)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = true;}
      
    if($this->idays == 1)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = false;}
    if($this->idays == 2)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = false;}
    if($this->idays == 3)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = false;}
    if($this->idays == 4)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = false;}
    if($this->idays == 5)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = true;}
         
    return $result;
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
   *   This Section's room or NULL if none is defined.
   */
  public function getRoom()
  {
    return $this->room;
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

  public function getStartTime()
  {
    return $this->start;
  }

  public function getEndTime()
  {
    return $this->tend;
  }

  public function getM()
  {
    return $this->bdays[0];
  }

  public function getTu()
  {
    return $this->bdays[1];
  }

  public function getW()
  {
    return $this->bdays[2];
  }

  public function getTh()
  {
    return $this->bdays[3];
  }

  public function getF()
  {
    return $this->bdays[4];
  }

  public function getDay($i)
  {
    return $this->bdays[$i];
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
   *   Get an array of information needed by the AJAX stuff.
   */
  public function to_json_array()
  {
    static $daymap = array(0 => 'm', 1 => 't', 2 => 'w', 3 => 'h', 4 => 'f');

    $json_array = array('section' => $this->letter,
			'prof' => $this->prof,
			'time_start' => $this->start,
			'time_end' => $this->tend,
			'days' => array(),
			'synonym' => $this->synonym,
			'room' => $this->room,
			);
    for ($day = 0; $day < 5; $day ++)
      $json_array['days'][$daymap[$day]] = $this->getDay($day);

    return $json_array;
  }
}
