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
   */
  function __construct ($letter, $time_start, $time_end, $days, $prof = '')
  {
    $this->letter = $letter;
    $this->start = $time_start;
    $this->tend = $time_end;

    $this->idays = $days;
    $this->bdays = $this->setbdays();

    $this->prof = $prof;
  }

  function setbdays()
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

  function getLetter()
  {
    return $this->letter;
  }

  function getProf()
  {
    return $this->prof;
  }

  function getStartTime()
  {
    return $this->start;
  }

  function getEndTime()
  {
    return $this->tend;
  }

  function getM()
  {
    return $this->bdays[0];
  }

  function getTu()
  {
    return $this->bdays[1];
  }

  function getW()
  {
    return $this->bdays[2];
  }

  function getTh()
  {
    return $this->bdays[3];
  }

  function getF()
  {
    return $this->bdays[4];
  }

  function getDay($i)
  {
    return $this->bdays[$i];
  }
  
  /**
   * \brief
   *   Create output suitable for editing on input.php.
   *
   * \see Classes::input_form_render()
   *
   * \param $class_key
   *   The same $class_key passed to Classes::input_form_render().
   * \param $section_key
   *   The index of this section.
   * \param $section_format
   *   The type of input method used for this section. Valid values
   *   are 'numerous', 'numbered', and 'lettered'
   */
  function input_form_render($class_key, $section_key, $section_format = 'numerous')
  {
    static $n = "\n";
    $out = '<tr class="section class' . $class_key . '">' . $n
      . '  <td class="none"></td>' . $n;
    switch ($section_format)
      {
      case 'numerous':
      default:
	/* see customIds() in scheduleInput.js */
	$out .= '  <td class="sectionIdentifier center">' . $n
	. '    <input type="text" size="1" class="required" title="Section Name"' . $n
	. '           name="postData[' . $class_key . '][' . $section_key . '][letter]"' . $n
	. '           value="' . htmlentities($this->letter) . '" />' . $n
	. "  </td>\n";
      break;
      }

    $out .= "  <td>\n"
      . '    <select class="selectRequired" name="postData[' . $class_key . '][' . $section_key . '][start]">' . $n;
    for ($h = 7; $h <= 21; $h ++)
      {
	$nm = 'p';
	$hr = $h;
	if ($h < 12)
	  $nm = 'a';
	elseif ($h > 12)
	  $hr -= 12;

	foreach (array('00', '30') as $m)
	  {
	    $val = $h . $m;

	    $selected = '';
	    if ($this->start == $val)
	      $selected = ' selected="selected"';

	    $label = $hr . ':' . $m . $nm . 'm';
	    $out .= '      <option value="' . $val . '"' . $selected . '>' . $label . '</option>' . $n;
	  }
      }
    $out .= "    </select>\n"
      . "  </td>\n";

    /* ugh, code duplication :-(  --binki commenting on his own code*/
    $out .= "  <td>\n"
      . '    <select class="selectRequired" name="postData[' . $class_key . '][' . $section_key . '][end]">' . $n;
    for ($h = 7; $h <= 21; $h ++)
      {
	$nm = 'p';
	$hr = $h;
	if ($h < 12)
	  $nm = 'a';
	elseif ($h > 12)
	  $hr -= 12;

	foreach (array('20', '50') as $m)
	  {
	    $val = $h . $m;

	    $selected = '';
	    if ($this->tend == $val)
	      $selected = ' selected="selected"';

	    $label = $hr . ':' . $m . $nm . 'm';
	    $out .= '      <option value="' . $val . '"' . $selected . '>' . $label . '</option>' . $n;
	  }
      }
    $out .= "    </select>\n"
      . "  </td>\n";

    foreach ($this->bdays as $day_key => $day_enabled)
      {
	if ($day_enabled)
	  $day_enabled = 'checked="checked"';
	else
	  $day_enabled = '';
	$out .= "  <td>\n"
	  . '    <input type="checkbox" class="daysRequired"'
	  . '           name="postData[' . $class_key . '][' . $section_key . '][days][' . $day_key . ']" value="1" ' . $day_enabled . ' />' . $n
	  . "  </td>\n";
      }

    $out .= '  <td><div class="deleteSection"><input type="button" value="X" class="gray" /></div></td>' . $n;
    $out .= '  <td></td>' . $n;

    $out .= "</tr>\n";

    return $out;
  }

  /**
   * \brief
   *   Splits up a section specifier into dept, course number, and
   *   section.
   *
   * For example, will return array('CS', '262', 'A') for 'CS-262-A'
   * or 'CS262A' or 'cs-262a'. This function is not for dealing with
   * course synonyms.
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

    if (!preg_match(';([0-9]+)[^a-zA-Z0-9]*;', $section_spec, $course_matches))
      return $ret;

    /* skip gunk */
    $section_spec = trim(substr($section_spec, strlen($course_matches[0])));
    $ret['course'] = $course_matches[1];

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
    static $daymap = array(0 => 'm', 1 => 't', 2 => 'w', 3 => 'u', 4 => 'f');

    $json_array = array('section' => $this->letter,
			'prof' => $this->prof,
			'time_start' => $this->start,
			'time_end' => $this->tend,
			'days' => array(),
			);
    for ($day = 0; $day < 5; $day ++)
      $json_array['days'][$daymap[$day]] = $this->getDay($day);

    return $json_array;
  }
}
