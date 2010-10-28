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
  function addSection($course_name, $letter, $time_start, $time_end, $days, $synonym = NULL, $faculty = NULL, $location = NULL)
  {
    $found = false;
    $counter = 0;
      
    while(!$found && ($counter < $this->nclasses))
      {
	$temp = $this->classStorage[$counter]->getName();
			
	if((strcmp($temp,$course_name)) == 0)
	  {
	    $found = true;
	  } else {
	  $counter++;
	}
      }
		
    if($counter == $this->nclasses)
      {
	echo 'Could not find class: ' . $course_name . "<br />\n";
      } else {
      $section = $this->classStorage[$counter]->section_get($letter);
      if (!$section)
	{
	  $section = new Section($letter, array(), $synonym, $faculty);
	  $this->classStorage[$counter]->section_add($section);
	}
      $section->meeting_add(new SectionMeeting($days, $time_start, $time_end, $location));
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
      $headcode = array('jQuery', 'jQueryUI', 'uiTabsKeyboard', 'outputStyle', 'outputPrintStyle', 'displayTables');
    }
    else {
      $headcode = array('outputStyle',  'jQuery', 'jQueryUI', 'uiTabsKeyboard', 'displayTables');
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
	  echo 'jQuery(\'#tabs-'.$item.'\').show();';
	}
      }
      echo '}); '; /* Close document.ready for jquery */
      echo 'window.print(); </script>';

      echo '<p><a href="'.$_SERVER['SCRIPT_NAME'].'?s=' . $this->id_get() . '">&laquo; Return to normal view</a> </p>';

    }
    else {
      echo '<script type="text/javascript">';
      echo 'jQuery(document).ready( function() {';
      echo 'jQuery("#tabs").tabs();';
      echo 'jQuery("#sharedialog").dialog({ modal: true, width: 550, resizable: false, draggable: false, autoOpen: false });';
      echo 'jQuery("#share").click( function() {
              jQuery("#sharedialog").dialog("open");
            });';
      echo 'jQuery(\'#printItems\').click( function() {
		window.location = "'.$_SERVER['SCRIPT_NAME'].'?s='.$this->id_get().'&amp;print=" + (jQuery(\'#tabs\').tabs(\'option\',\'selected\') + 1);
	    });
	    jQuery(\'#cancelItems\').click( function() {
		jQuery(\'#selectItemsInput\').hide();
	    });
';
      echo '});</script>'; /* Close document.ready for jquery */
      echo '<div id="sharedialog" title="Share Schedule"><p>You can share your schedule with the URL below:</p><p><!--http://' . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'].'-->'.$outputPage->gen_share_url($this->id_get()).'</p></div>';
      echo '<p><span id="printItems"><a href="#">Print</a></span> :: <span id="share"><a href="#">Share</a></span> :: <a href="input.php">Home</a></p><p class="centeredtext">Having problems? <a href="feedback.php">Let us know</a>.</p><p class="centeredtext graytext"><em>Keyboard Shortcut: Left and right arrow keys switch between schedules</em></p>';
    }		

    echo "\n";

    if($this->nPermutations > 0)
      {
	$table .= "<div id=\"tabs\">\n"


    . '  <div id="show-box" class="show-buttons">
    <form>
       <label><strong>Display:</strong></label>
       <input id="show-prof" name="show-prof" type="checkbox" checked="checked" /><label for="show-prof">Professor</label>
       <input id="show-location" name="show-location" type="checkbox" /><label for="show-location">Room</label>
       <input id="show-synonym" name="show-synonym" type="checkbox" /><label for="show-synonym">Synonym</label>
    </form>
  </div> <!-- id="show-box" -->'




	  . "<div><ul>\n";
			
	for($nn = 1; $nn <= $this->nPermutations; $nn++)
	  {
	    $table .= "<li><a href=\"#tabs-" . $nn . "\">&nbsp;" . $nn . "&nbsp;</a></li>\n";
	  }
			
	$table .= "    </ul></div>\n  \n"
	  . "  <div class=\"scroller\">\n"
	  . "    <div class=\"scontent\">\n";
		
	for($i = 0; $i < $this->nPermutations; $i++)
	  {
	    $table .= '      <div class="section" id="tabs-' . ($i+1) . "\">\n";
  
	    // Beginning of table
	    $table .= "        <table style=\"empty-cells:show;\" border=\"1\" cellspacing=\"0\">\n";
				
	    // Header row
	    $table .= "          <tr>\n"
	      . '            <td class="none permuteNum">' . ($i + 1) . "</td>\n"
	      . "            <td class=\"day\">Monday</td>\n"
	      . "            <td class=\"day\">Tuesday</td>\n"
	      . "            <td class=\"day\">Wednesday</td>\n"
	      . "            <td class=\"day\">Thursday</td>\n"
	      . "            <td class=\"day\">Friday</td>\n"
	      . "          </tr>\n";

	    $last_meeting = array();
	    $rowspan = array(0, 0, 0, 0, 0);
	    for($r = 0; $r < (count($time)-1); $r++)
	      {

		$table .= "          <tr>\n"
		  . "            <td class=\"time\">" . $this->prettyTime($time[$r]) . "</td>\n";

		for($dayLoop = 0; $dayLoop < 5; $dayLoop++)
		{
		  /* Makes sure there is not a class already in progress */
		  if($rowspan[$dayLoop] <= 0)
		    {
		      for($j = 0; $j < $this->nclasses; $j++)
			{
			  $class = $this->classStorage[$j];
			  $section_index = $this->storage[$i][$j];
			  $section = $class->getSection($section_index);
				  /* iterate through all of a class's meeting times */
				  $meetings = $section->getMeetings();

				  /* find any meeting which are going on at this time */
				  $current_meeting = NULL;
				  foreach ($meetings as $meeting)
				    {
				      if ($meeting->getDay($dayLoop)
					  && $meeting->getStartTime() >= $time[$r]
					  && $meeting->getStartTime() < $time[$r+1])
					{
					  $current_meeting = $meeting;
					}
				    }
				  
				  if ($current_meeting)
				    {
				      /* calculate how many rows this section should span */
				      for ($my_r = $r; $current_meeting->getEndTime() > $time[$my_r]; $my_r ++)
					;
				      $rowspan[$dayLoop] = $my_r - $r;

				      $single_multi = 'single';
				      if ($rowspan[$dayLoop] > 1)
					$single_multi = 'multi';

				      $table .= '            <td rowspan="' . $rowspan[$dayLoop]
					. '" class="' . $single_multi . ' class' . $j
					. '" title="prof: ' . htmlentities($section->getProf(), ENT_QUOTES)
					. ', room: ' . htmlentities($current_meeting->getLocation(), ENT_QUOTES) . '">'
					. htmlentities($class->getName(), ENT_QUOTES) . '-'
					. htmlentities($section->getLetter(), ENT_QUOTES) . "\n"
					. '<span class="prof block">' . htmlentities($section->getProf(), ENT_QUOTES) . "</span>\n"
					. '<span class="location block">' . htmlentities($current_meeting->getLocation(), ENT_QUOTES) . "</span>\n"
					. '<span class="synonym block">' . htmlentities($section->getSynonym(), ENT_QUOTES) . "</span>\n"
					. "</td>\n";
				      $filled = TRUE;
				    }
			}
		    }

		  if ($rowspan[$dayLoop] > 0)
		    {
		      $filled = TRUE;
		      $rowspan[$dayLoop] --;
		    }

		  /* If the cell was not filled, fill it with an empty cell. */
			if(!$filled)
			{
				$table .= "            <td class=\"none\">&nbsp;</td>\n";
			}
			$filled = FALSE;
		}
		
		// End of row
		$table .= "          </tr>\n";
	      }

	    // End of table
	    $table .= "        </table>\n"
	      . '      </div> <!-- id="section' . ($i + 1) . "\" -->\n";
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
      echo '<form method="get" action="input.php"><p><input type="hidden" name="s" value="' . $id . '" /><input class="gray" id="editbutton" type="submit" value="edit" /></p></form>';

    echo "<p id=\"possiblestats\">There were a total of " . $this->possiblePermutations . " possible permutations. Only " . $this->nPermutations . " permutations had no class conflicts.</p>";

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
}
