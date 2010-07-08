<?php

//**************************************************
// class.class.php	Author: Nathan Gelderloos
//
// Represents a class.
//**************************************************

include_once 'errors.php';
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
}

?>