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
    $this->sections = array();
    $this->name = $n;
    $this->nsections = 0;
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
    foreach ($this->sections as $section)
      if ($section->getLetter() == $letter)
	return $section;

    return NULL;
  }

  //--------------------------------------------------
  // Returns the name of the class.
  //--------------------------------------------------
  public function getName()
  {
    return $this->name;
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
	$section_json_arrays = $section->to_json_arrays();
	foreach ($section_json_arrays as $section_json_array)
	  $json_array['sections'][] = $section_json_array;
      }

    return $json_array;
  }
}
