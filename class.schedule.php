<?php

//**************************************************
// class.schedule.php	Author: Nathan Gelderloos
//
// Represents a schedule of a week. Stores the
// classes that are part of that week and calculates
// all the possible permutations.
//**************************************************

include_once 'class.class.php';
include_once 'class.section.php';
include_once 'inc/class.page.php';

class Schedule
{
  private $classStorage;			// Classes array of classes
  private $nclasses;				// Integer number of classes
  private $nPermutations = 0;		// Integer number of real permutations
  private $possiblePermutations;	// Integer number of possible permutations
  private $scheduleName;			// String name of schedule
  private $storage;				// Integer array of valid schedules
  private $title;

	private $classContinue = array(-1, -1, -1, -1, -1);
  
  /**
   * \brief
   *   My global identification number. Not defined until the schedule
   *   is processed and first saved.
   */
  private $id;
  /**
   * The input format of the sections. Only used for the UI. Valid
   * values are 'numerous' for custom, 'numbered' for numeric, and 'lettered' for
   * alphabetical.
   */
  public $section_format;

  /**
   * \brief
   *   Create a schedule with the given name.
   */
  function __construct($name)
  {
    $this->classStorage = array();
    $this->nclasses = 0;
    $this->scheduleName = $name;
    $this->storage = array();
    $this->title = "SlatePermutate - Scheduler";
    $this->section_format = 'numerous';
  }

  //--------------------------------------------------
  // Mutators and Accessors
  //--------------------------------------------------
  public function getName()
  {
    return $this->scheduleName;
  }    

  //--------------------------------------------------
  // Adds a new class to the schedule.
  //--------------------------------------------------
  function addClass($n)
  {
    $this->classStorage[$this->nclasses] = new Classes($n);
    $this->nclasses++;
  }

  //--------------------------------------------------
  // Adds a section to the desired class.
  //--------------------------------------------------
  function addSection($n, $l, $s, $e, $d, $synonym = NULL, $faculty = NULL, $room = NULL)
  {
    $found = false;
    $counter = 0;
      
    while(!$found && ($counter < $this->nclasses))
      {
	$temp = $this->classStorage[$counter]->getName();
			
	if((strcmp($temp,$n)) == 0)
	  {
	    $found = true;
	  } else {
	  $counter++;
	}
      }
		
    if($counter == $this->nclasses)
      {
	echo "Could not find class: " . $n . "<br />";
      } else {
      $this->classStorage[$counter]->section_add(new Section($l, $s, $e, $d, $synonym, $faculty, $room));
    }
  }

  //--------------------------------------------------
  // Finds all of the possible permutations and stores
  // the results in the storage array.
  //--------------------------------------------------
	function findPossibilities()
	{
		$this->possiblePermutations = 1;
		/* special case: there is nothing entered into the schedule and thus there is one, NULL permutation */
		if (!$this->nclasses)
		{
			/* have an empty schedule */
			$this->classStorage[0] = array();
			$this->nPermutations = 1;
			return;
		}

		$position = 0;
		$counter = 0;

		for($i = 0; $i < $this->nclasses; $i++)
		{
			$this->possiblePermutations = $this->possiblePermutations * $this->classStorage[$i]->getnsections();
			$cs[$i] = 0;	// Sets the counter array to all zeroes.
		}
        
		// Checks for conflicts in given classes, stores if none found
		do
		{
			$conflict = false;
         
			// Get first class to compare
			for ($upCounter = 0; $upCounter < $this->nclasses && !$conflict; $upCounter ++)
			{
	    
				for ($downCounter = $this->nclasses - 1; $downCounter > $upCounter && !$conflict; $downCounter --)
				{
				  if ($this->classStorage[$upCounter]->getSection($cs[$upCounter])
				      ->conflictsWith($this->classStorage[$downCounter]->getSection($cs[$downCounter])))
				    {
				      $conflict = TRUE;
				      break;
				    }
				}
			}
	
	// Store to storage if no conflict is found.
	if(!$conflict)
	  {
	    for($i = 0; $i < $this->nclasses; $i++)
	      {
		$this->storage[$this->nPermutations][$i] = $cs[$i];
	      }
	    $this->nPermutations++;
	  }
			
	// Increase the counter by one to get the next combination of class sections.
	$cs[$position] = $cs[$position] + 1;
			
	// Check to make sure the counter is still valid.
	$valid = false;
	while(!$valid)
	  {
	    if($cs[$position] == $this->classStorage[$position]->getnsections())
	      {
		$cs[$position] = 0;

		$position++;
					
		// This is for the very last permutation. Even 
		// though the combination is not actually true
		// the larger while loop will end before any 
		// new combinations are performed.
		if($position == $this->nclasses)
		  {
		    $valid = true;
		  } else {
		  $cs[$position]++;
		}
	      } else {
	      $valid = true;
	      $position = 0;
	    }
	  }
            
	$counter++;
      } while($counter < $this->possiblePermutations);
  }
    
  //--------------------------------------------------
  // Prints out the possible permutations in tables.
  //--------------------------------------------------
  function writeoutTables()
  {
    $table = "";
    $filled = false;
    $time = array(700,730,800,830,900,930,1000,1030,1100,1130,1200,1230,1300,1330,1400,1430,1500,1530,1600,1630,1700,1730,1800,1830,1900,1930,2000,2030,2100,2130, 2200);

    $footcloser = '';

    if(isset($_REQUEST['print']) && $_REQUEST['print'] != ''){
      $headcode = array('jQuery', 'jQueryUI', 'uiTabsKeyboard', 'outputStyle', 'outputPrintStyle');
    }
    else {
      $headcode = array('outputStyle',  'jQuery', 'jQueryUI', 'uiTabsKeyboard');
    }
    $outputPage = new Page(htmlentities($this->getName()), $headcode);



    if(isset($_REQUEST['print'])){
 
     echo '<script type="text/javascript">';
      echo 'jQuery(document).ready( function() {';
 
      /* If user entered items to print */
      if($_REQUEST['print'] != 'all'){
	echo 'jQuery(\'.section\').hide();';
	$items = explode(',', $_REQUEST['print']);
	foreach($items as $item){
	  echo 'jQuery(\'#section'.$item.'\').show();';
	}
      }

      echo 'jQuery(\'#selectItemsInput\').hide();
			      jQuery(\'#selectItems\').click( function() {
				jQuery(\'#selectItemsInput\').show();
			      });
			      jQuery(\'#cancelItems\').click( function() {
				jQuery(\'#selectItemsInput\').hide();
			      });';
      echo '}); '; /* Close document.ready for jquery */
      echo 'window.print(); </script>';

      echo '<p><span id="selectItems"><a href="#">Select Schedules to Print</a></span> :: <a href="'.$_SERVER['SCRIPT_NAME'].'?s=' . $this->id_get() . '">Return to normal view</a> :: <a href="input.php">Home</a></p>';
      echo '<div  id="selectItemsInput"><p><form action="'.$_SERVER["SCRIPT_NAME"].'?s=' . $this->id_get() . '"><label><strong>Schedules to Print</strong> <em>(seperate with commas, "all" for all)</em></label><br /><input type="text" name="print" value="'.$_REQUEST['print'].'" /><input type="submit" value="submit" /><span id="cancelItems"><input type="button" value="cancel" /></span></form></p></div>';
    }
    else {
      echo '<script type="text/javascript">';
      echo 'jQuery(document).ready( function() {';
      echo 'jQuery("#tabs").tabs();';
      echo '});</script>'; /* Close document.ready for jquery */
      echo '<p><a href="'.$_SERVER["SCRIPT_NAME"].'?s=' . $this->id_get() . '&amp;print=all">Print</a> :: <a href="input.php">Home</a></p><p class="centeredtext" style="color: #999;"><em>Keyboard Shortcut: Left and right arrow keys switch between schedules</em></p>';
    }		

    if($this->nPermutations > 0)
      {
	$table .= "<div id=\"tabs\">\n"
	  . "  <ul>\n";
			
	for($nn = 1; $nn <= $this->nPermutations; $nn++)
	  {
	    $table .= "<li><a href=\"#tabs-" . $nn . "\">&nbsp;" . $nn . "&nbsp;</a></li>\n";
	  }
			
	$table .= "    </ul><div class=\"clear\"><p> </p> </div>\n  \n"
	  . "  <div class=\"scroller\">"
	  . "    <div class=\"scontent\">";
		
	for($i = 0; $i < $this->nPermutations; $i++)
	  {
	    $table .= "<div class=\"section\" id=\"tabs-" . ($i+1) . "\">";
  
	    // Beginning of table
	    $table .= "<table style=\"empty-cells:show;\" border=\"1\" cellspacing=\"0\">";
				
	    // Header row
	    $table .= "\n\t<tr>\n\t\t<td class=\"none permuteNum\">" . ($i+1) . "</td>\n\t\t<td class=\"day\">Monday</td>\n\t\t<td class=\"day\">Tuesday</td>\n\t\t<td class=\"day\">Wednesday</td>\n\t\t<td class=\"day\">Thursday</td>\n\t\t<td class=\"day\">Friday</td>\n\t</tr>";

	    for($r = 0; $r < (count($time)-1); $r++)
	      {

		$table .= "\n\t<tr>\n\t\t<td class=\"time\">" . $this->prettyTime($time[$r]) . "</td>";

		for($dayLoop = 0; $dayLoop < 5; $dayLoop++)
		{
			for($j = 0; $j < $this->nclasses; $j++)
			{
				// Makes sure there is not a class already in progress
				if($this->getClassCont($dayLoop) == -1)
				{
					// Checks if the class meets on the given day
					if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getDay($dayLoop)))
					{
						// Checks if the class meets at the given time
						if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
						{
							// Checks if the class continues after the given time
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
							{
								$table .= "\n\t\t<td class=\"top class{$j}\">" . htmlentities($this->classStorage[$j]->getName()) . " " . htmlentities($this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter()) . "</td>";
								$this->setClassCont($dayLoop, $j);
								$filled = true;
							}else{
								$table .= "\n\n\t<td class=\"single class{$j}\">" . htmlentities($this->classStorage[$j]->getName()) . " " . htmlentities($this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter()) . "</td>";
								$filled = true;
							}
						}
					}
				}else{
					if($j == $this->getClassCont($dayLoop))
					{
						if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
						{
							$table .= "\n\t\t<td class=\"mid class{$j}\">&nbsp;</td>";
							$filled = true;
						}else{
							$table .= "\n\t\t<td class=\"end class{$j}\">&nbsp;</td>";
							$this->setClassCont($dayLoop, -1);
							$filled = true;
						}
					}
				}
			}
			
			// If the cell was not filled, fill it with an empty cell.
			if(!$filled)
			{
				$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
			}
			$filled = false;
		}
		
		// End of row
		$table .= "\n\t</tr>";
	      }

	    // End of table
	    $table .= '</table></div> <!-- id="section' . ($i + 1) . "\" -->\n";
	  }

	echo $table
	  . "    </div> <!-- class=\"scontent\" -->\n"
	  . "  </div> <!-- class=\"scroller\" -->\n"
	  . "</div> <!-- id=\"my-glider\" -->\n"
	  . $footcloser; // Closes off the content div
      } else {
      echo '<html><body><p>There are no possible schedules. Please try again.</p></body></html>';
    }

    /* edit button */
    if ($id = $this->id_get())
      echo '<form method="get" action="input.php"><p><input type="hidden" name="s" value="' . $id . '" /><input type="submit" value="edit" /></p></form>';

    echo "<p>There were a total of " . $this->possiblePermutations . " possible permutations. Only " . $this->nPermutations . " permutations had no class conflicts.</p>";

    $outputPage->foot();
  }

  //--------------------------------------------------
  // Changes the title of the page.
  //--------------------------------------------------
  function changeTitle($t)
  {
    $this->title = $t;
  }

  //--------------------------------------------------
  // Make the time "pretty."
  //--------------------------------------------------
  function prettyTime($t){
    if($t > 1259)
      {
	$t = ($t-1200);
	return substr($t, 0, strlen($t)-2) . ":" . substr($t, strlen($t)-2, strlen($t)) . " PM";
      } else {
      return substr($t, 0, strlen($t)-2) . ":" . substr($t, strlen($t)-2, strlen($t)) . " AM";
    }
  }

  /**
   * \brief
   *   fetch the number of classes
   */
  function nclasses_get()
  {
    return $this->nclasses;
  }

  /*
   * \brief
   *   fetch a specified class by its key
   */
  function class_get($class_key)
  {
    return $this->classStorage[$class_key];
  }

  /**
   * \brief
   *   Set my global ID.
   *
   * Only to be called by schedule_store_store().
   */
  function id_set($id)
  { 
    $this->id = $id;
  }

  /*
   * \brief
   *   Get my global ID.
   */
  function id_get()
  {
    return $this->id;
  }

  
	function getClassCont($day)
	{
		return $this->classContinue[$day];
	}
	
	function setClassCont($day, $i)
	{
		$this->classContinue[$day] = $i;
	}
}
