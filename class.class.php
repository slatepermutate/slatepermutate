<?php

//**************************************************
// class.class.php	Author: Nathan Gelderloos
//
// Represents a class.
//**************************************************

include_once 'class.section.php';

class Classes
{
  private $name;		// String
  private $sections;	// Array of sections
  private $nsections;	// int
    
  //--------------------------------------------------
  // Creates a class with the given name.
  //--------------------------------------------------
  function __construct($n)
  {
    $this->name = $n;
    $this->nsections = 0;
  }
	
  //--------------------------------------------------
  // Adds a new section to the class.
  //--------------------------------------------------
  function addSection($l, $p, $s, $e, $d)
  {
    $this->sections[$this->nsections] = new Section($l, $p, $s, $e, $d);
    $this->nsections++;
  }

  /**
   * \brief
   *   Adds an already-instantiated section to this class.
   */
  public function section_add(Section $section)
  {
    $this->sections[$this->nsections] = $section;
    $this->nsections ++;
  }

  //--------------------------------------------------
  // Returns the number of sections in the class.
  //--------------------------------------------------
  function getnsections()
  {
    return $this->nsections;
  }
	
  //--------------------------------------------------
  // Returns the desired section for analysis.
  //--------------------------------------------------
  function getSection($i)
  {
    // Checks to make sure the desired section is part of the set.
    if(isset($this->sections[$i]))
      {
	//echo "Object sections[$i] was set<br />";
      } else {
      echo "Object sections[$i] was NOT set <br />";
    }

    $result = $this->sections[$i];
    return $result;
  }
    
  //--------------------------------------------------
  // Returns the name of the class.
  //--------------------------------------------------
  function getName()
  {
    return $this->name;
  }

  /**
   * \brief
   *   Renders this Classes into something suitable for input.php.
   */
  function input_form_render($class_key)
  {
    $n = "\n";
    $out = '<tr title="' . $class_key . '" class="class class' . $class_key . '">' . $n
      . '  <td><input type="text" class="required defText" title="Class Name" name="postData[' . $class_key . '][name]" value="' . htmlentities($this->getName()) . '"/></td>' . $n
      . '  <td colspan="8"></td>' . $n
      . '  <td class="tdInput"><div class="addSection"><input type="button" value="Add section" class="gray" /></div></td>' . $n
      . '  <td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" class="gray" /></div></td>' . $n
      . "</tr>\n";

    foreach ($this->sections as $key => $section)
      $out .= $section->input_form_render($class_key, $key);

    return $out;
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
   *   Represent this class as a string.
   */
  public function __toString()
  {
    return $this->getName();
  }

  /**
   * \brief
   *   Represent this class as an array of sections ready to be JSONized.
   */
  public function to_json_array()
  {
    $json_array = array('class' => $this->getName(),
			'sections' => array());
    foreach ($this->sections as $section)
      {
	$json_array['sections'][] = $section->to_json_array();
      }

    return $json_array;
  }
}
