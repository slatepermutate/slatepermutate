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
      . '  <td><input type="text" class="required defText" title="Class Name" name="postData[' . $class_key . '][name]" value="' . str_replace('"', '&quot;', $this->getName()) . '"/></td>' . $n
      . '  <td colspan="8"></td>' . $n
      . '  <td class="tdInput"><div class="addSection"><input type="button" value="Add section" /></div></td>' . $n
      . '  <td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" /></div></td>' . $n
      . "</tr>\n";

    foreach ($this->sections as $key => $section)
      $out .= $section->input_form_render($class_key, $key);

    return $out;
  }
}
