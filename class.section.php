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

  function __construct ($l, $p, $s, $e, $d)
  {
    $this->letter = $l;
    $this->prof = $p;
    $this->start = $s;
    $this->tend = $e;
    $this->idays = $d;
    $this->bdays = $this->setbdays();
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
}
