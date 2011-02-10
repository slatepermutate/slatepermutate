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

//**************************************************
// class.schedule.php	Author: Nathan Gelderloos
//
// Represents a schedule of a week. Stores the
// classes that are part of that week and calculates
// all the possible permutations.
//**************************************************

$incdir = dirname(__FILE__) . DIRECTORY_SEPARATOR;
include_once $incdir . 'class.course.inc';
include_once $incdir . 'class.section.php';
include_once $incdir . 'class.page.php';

/*
 * Load a Classes -> Course converter class for the sake of the
 * Schedule::__wakeup() magic function.
 */
require_once $incdir . 'class.classes_convert.inc';

class Schedule
{
  /*
   * Variables for upgrading from saved schedules created when there
   * was a class called Classes.
   */
  private $classStorage;			// array of courses
  private $nclasses;				// Integer number of classes

  /* My member variables. */
  private $courses;
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
    $this->courses = array();
    $this->scheduleName = $name;
    $this->storage = array();
    $this->title = "SlatePermutate - Scheduler";
    $this->section_format = 'numerous';

    /* mark this as an upgraded Schedule class. See __wakeup() */
    $this->nclasses = -1;
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
  function addCourse($n)
  {
    $this->courses[] = new Course($n);
  }

  //--------------------------------------------------
  // Adds a section to the desired class.
  //--------------------------------------------------
  function addSection($course_name, $letter, $time_start, $time_end, $days, $synonym = NULL, $faculty = NULL, $location = NULL, $type = 'lecture')
  {
    $found = false;
    $counter = 0;

    foreach ($this->courses as $course)
      if (!strcmp($course_name, $course->getName()))
	{
	  $section = $course->section_get($letter);
	  if (!$section)
	    {
	      $section = new Section($letter, array(), $synonym, $faculty);
	      $course->section_add($section);
	    }
	  $section->meeting_add(new SectionMeeting($days, $time_start, $time_end, $location, $type));

	  return;
	}

    echo 'Could not find class: ' . $course_name . "<br />\n";
  }

  //--------------------------------------------------
  // Finds all of the possible permutations and stores
  // the results in the storage array.
  //--------------------------------------------------
	function findPossibilities()
	{
		$this->possiblePermutations = 1;
		/* special case: there is nothing entered into the schedule and thus there is one, NULL permutation */
		if (!count($this->courses))
		{
			/* have an empty schedule */
			$this->nPermutations = 1;
			return;
		}

		$position = 0;
		$counter = 0;

		$i = 0;
		foreach ($this->courses as $course)
		{
			$this->possiblePermutations = $this->possiblePermutations * $course->getnsections();
			$cs[$i] = 0;	// Sets the counter array to all zeroes.
			$i ++;
		}
        
		// Checks for conflicts in given classes, stores if none found
		do
		{
			$conflict = false;
         
			// Get first class to compare
			for ($upCounter = 0; $upCounter < count($this->courses) && !$conflict; $upCounter ++)
			{
	    
			  for ($downCounter = count($this->courses) - 1; $downCounter > $upCounter && !$conflict; $downCounter --)
				{
				  if ($this->courses[$upCounter]->getSection($cs[$upCounter])
				      ->conflictsWith($this->courses[$downCounter]->getSection($cs[$downCounter])))
				    {
				      $conflict = TRUE;
				      break;
				    }
				}
			}
	
	// Store to storage if no conflict is found.
	if(!$conflict)
	  {
	    for($i = 0; $i < count($this->courses); $i++)
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
	    if($cs[$position] == $this->courses[$position]->getnsections())
	      {
		$cs[$position] = 0;

		$position++;
					
		// This is for the very last permutation. Even 
		// though the combination is not actually true
		// the larger while loop will end before any 
		// new combinations are performed.
		if($position == count($this->courses))
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
    $filled = false;
    $time = array(700,730,800,830,900,930,1000,1030,1100,1130,1200,1230,1300,1330,1400,1430,1500,1530,1600,1630,1700,1730,1800,1830,1900,1930,2000,2030,2100,2130, 2200);

    define('SP_PERMUTATIONS_PER_PAGE', 256); /** @TODO: Define this in config.inc */

    $npages = ceil($this->nPermutations / SP_PERMUTATIONS_PER_PAGE);
    $page = 0;
    if (isset($_REQUEST['page']))
      $page = $_REQUEST['page'];
    /*
     * only display the ``this page doesn't exist'' 404 if there is at
     * least one permutation. Otherwise, we give an irrelevant 404 for
     * users with no permutations.
     */
    if ($this->nPermutations && $page >= $npages)
      Page::show_404('Unable to find page ' . $page . ', there are only ' . $this->nPermutations . ' non-conflicting permutations, for a total of ' . $npages . ' pages.');
    /* zero-based */
    $first_permutation = $page * SP_PERMUTATIONS_PER_PAGE;
    $last_permutation = min($this->nPermutations, $first_permutation + SP_PERMUTATIONS_PER_PAGE);

    $footcloser = '';

    if(isset($_REQUEST['print']) && $_REQUEST['print'] != ''){
      $headcode = array('jQuery', 'jQueryUI', 'uiTabsKeyboard', 'outputStyle', 'outputPrintStyle', 'displayTables');
    }
    else {
      $headcode = array('outputStyle',  'jQuery', 'jQueryUI', 'uiTabsKeyboard', 'displayTables');
    }
    $outputPage = page::page_create(htmlentities($this->getName()), $headcode);
    $outputPage->head();



    if(isset($_REQUEST['print'])) {
 
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
      echo '}); '; /* Close document.ready for jQuery */
      echo 'window.print(); </script>';

      echo '<p><a href="'.$_SERVER['SCRIPT_NAME'].'?s=' . $this->id_get() . '">&laquo; Return to normal view</a> </p>';

    }
    else {
      echo '<script type="text/javascript">';
      echo '  jQuery(document).ready( function() {';
      echo '    jQuery("#tabs").tabs();';
      echo '    jQuery("#sharedialog").dialog({ modal: true, width: 550, resizable: false, draggable: false, autoOpen: false });';
      echo '    jQuery("#share").click( function() {
                  jQuery("#sharedialog").dialog("open");
                });';
      echo '    jQuery(\'#printItems\').click( function() {
		  window.location = "'.$_SERVER['SCRIPT_NAME'].'?s='.$this->id_get().'&amp;print=" + (jQuery(\'#tabs\').tabs(\'option\',\'selected\') + 1);
	        });
	        jQuery(\'#cancelItems\').click( function() {
		  jQuery(\'#selectItemsInput\').hide();
	        });';
      echo '  });
            </script>';

      echo '<div id="sharedialog" title="Share Schedule"><p>You can share your schedule with the URL below:</p><p>' . htmlentities($outputPage->gen_share_url($this->id_get())) . '</p></div>' . "\n";
      echo '<p><a href="input.php?s='.$this->id.'">Edit</a> :: <span id="printItems"><a href="#">Print</a></span> :: <span id="share"><a href="#">Share</a></span> :: <a href="input.php">Home</a></p>'. "\n";
      echo '<p class="centeredtext">Having problems? <a href="feedback.php">Let us know</a>.</p>' . "\n";
      echo '<p class="centeredtext graytext"><em>Keyboard Shortcut: Left and right arrow keys switch between schedules</em></p>' . "\n";

    }		

    echo "\n";

    if($this->nPermutations > 0)
      {
	/*
	 * Figure out if we have to deal with Saturday and then deal
	 * with it.
	 *
	 * Also, ensure that our $time array is big enough for all of
	 * these courses.
	 */
	$max_day_plusone = 5;
	$have_saturday = FALSE;

	$max_time = (int)max($time);
	$min_time = (int)min($time);
	$sort_time = FALSE;
	foreach ($this->courses as $course)
	  {
	    for ($si = 0; $si < $course->getnsections(); $si ++)
	      foreach ($course->getSection($si)->getMeetings() as $meeting)
		{
		  /* Saturdayness */
		  if ($meeting->getDay(5))
		    {
		      $max_day_plusone = 6;
		      $have_saturday = TRUE;
		    }

		  /* very late / very early classes */
		  while ($meeting->getEndTime() > $max_time)
		    {
		      $max_time += 30;
		      $time[] = $max_time;
		    }
		  while ($meeting->getStartTime() < $min_time)
		    {
		      $min_time -= 30;
		      $time[] = $min_time;
		      $sort_time = TRUE;
		    }
		}
	  }
	/* ensure that early times are actually first ;-) */
	if ($sort_time)
	  sort($time);

        echo '<div id="regDialog" title="Registration Codes"><p>Enter these codes into your school\'s online course registration system to register for classes:</p><div id="regDialogList"></div></div>';
	echo '<div id="tabs">' . "\n" .
               '<div id="show-box" class="show-buttons">
                  <form action="#"><p class="nospace">
                    <label><strong>Display:</strong></label>
                    <input id="show-prof" name="show-prof" type="checkbox" checked="checked" /><label for="show-prof">Professor</label>
                    <input id="show-location" name="show-location" type="checkbox" /><label for="show-location">Room</label>
                    <input id="show-synonym" name="show-synonym" type="checkbox" /><label for="show-synonym">Synonym</label>
                    <span id="regCodes"><label><a href="#">Registration Codes</a></label></span></p>
                  </form>';

          echo '</div> <!-- id="show-box" -->'
	     . '<div id="the-tabs"><ul>' . "\n";
			
	for($nn = $first_permutation + 1; $nn <= $last_permutation; $nn++)
	  {
	    echo  "<li><a href=\"#tabs-" . $nn . "\">&nbsp;" . $nn . "&nbsp;</a></li>\n";
	  }
			
	echo "    </ul></div>\n  \n";

	echo "    <div id=\"pagers\">\n";
	/* Previous button */
	if ($page > 0)
	  echo '      <div id="pager-previous" class="pager left"><a href="' . htmlentities($this->url($this->id, $page - 1)) . '">&laquo; Previous</a></div>' . "\n";

	/* Next button */
	if ($page + 1 < $npages)
	  echo '      <div id="pager-next" class="pager right"><a href="' . htmlentities($this->url($this->id, $page + 1)) . '">Next &raquo;</a></div>' . "\n";
	echo "    </div> <!-- id=\"pagers\" -->\n";


	echo "  <div class=\"scroller\">\n"
	  . "    <div class=\"scontent\">\n";
		
	for($i = $first_permutation; $i < $last_permutation; $i++)
	  {
             $syns = array();
	     echo  '      <div class="section" id="tabs-' . ($i+1) . "\">\n";
  
	    // Beginning of table
	    echo "        <table style=\"empty-cells:show;\" border=\"1\" cellspacing=\"0\">\n";
				
	    // Header row
	    echo "          <tr>\n"
	      . '            <td class="none permuteNum">' . ($i + 1) . "</td>\n"
	      . "            <td class=\"day\">Monday</td>\n"
	      . "            <td class=\"day\">Tuesday</td>\n"
	      . "            <td class=\"day\">Wednesday</td>\n"
	      . "            <td class=\"day\">Thursday</td>\n"
	      . "            <td class=\"day\">Friday</td>\n";
	    if ($have_saturday)
	      echo "            <td class=\"day\">Saturday</td>\n";
	    echo "          </tr>\n";

	    $last_meeting = array();
	    $rowspan = array(0, 0, 0, 0, 0, 0);
	    for($r = 0; $r < (count($time)-1); $r++)
	      {

		echo "          <tr>\n"
		  . "            <td class=\"time\">" . $this->prettyTime($time[$r]) . "</td>\n";

		/* currently, 0-5 = monday-saturday */
		for($dayLoop = 0; $dayLoop < $max_day_plusone; $dayLoop++)
		{
		  /* Makes sure there is not a class already in progress */
		  if($rowspan[$dayLoop] <= 0)
		    {
		      for($j = 0; $j < count($this->courses); $j++)
			{
			  $class = $this->courses[$j];
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
				      for ($my_r = $r;
					   $my_r < (count($time)-1) && $current_meeting->getEndTime() > $time[$my_r];
					   $my_r ++)
					;
				      $rowspan[$dayLoop] = $my_r - $r;

				      $single_multi = 'single';
				      if ($rowspan[$dayLoop] > 1)
					$single_multi = 'multi';

				      echo '            <td rowspan="' . $rowspan[$dayLoop]
					. '" class="' . $single_multi . ' class' . $j
					. '" title="prof: ' . htmlentities($section->getProf(), ENT_QUOTES)
					. ', room: ' . htmlentities($current_meeting->getLocation(), ENT_QUOTES)
					. ', type: ' . htmlentities($current_meeting->type_get(), ENT_QUOTES) . '">'
					. htmlentities($class->getName(), ENT_QUOTES) . '-'
					. htmlentities($section->getLetter(), ENT_QUOTES) . "\n"
					. '<span class="prof block">' . htmlentities($section->getProf(), ENT_QUOTES) . "</span>\n"
					. '<span class="location block">' . htmlentities($current_meeting->getLocation(), ENT_QUOTES) . "</span>\n"
					. '<span class="synonym block">' . htmlentities($section->getSynonym(), ENT_QUOTES) . "</span>\n"
					. "</td>\n";
				      $syns[$section->getSynonym()] = $section->getSynonym();
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
				echo "            <td class=\"none\">&nbsp;</td>\n";
			}
			$filled = FALSE;
		}
		
		// End of row
		echo "          </tr>\n";
	      }



	    // End of table
	    echo "        </table>\n"
              . '         <span class="syns syns'.$i.'">'.  json_encode($syns) . "</span>\n"
	      . '      </div> <!-- id="section' . ($i + 1) . "\" -->\n";
	  }

          echo "    </div> <!-- class=\"scontent\" -->\n"
	     . "  </div> <!-- class=\"scroller\" -->\n"
	     . "</div> <!-- id=\"my-glider\" -->\n"
	     . $footcloser; // Closes off the content div
      } else {
      echo '<html><body><p>There are no possible schedules. Please <a href="input.php?s='.$this->id.'">try again</a>.</p></body></html>';
    }

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
  // Make the time "pretty"
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
    return count($this->courses);
  }

  /**
   * \brief
   *   fetch a specified class by its key
   */
  function class_get($class_key)
  {
    return $this->courses[$class_key];
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

  /**
   * \brief
   *   Write out a relative URL for a particular schedule.
   *
   * Takes into account the $clean_urls setting.
   *
   * \param $id
   *   The ID of the schedule to link to. Defaults to the current schedule object.
   * \param $page
   *   The page of the schedule to link to. Defaults to 0.
   * \return
   *   A string, the URL used to access this schedule. Remember that
   *   if this string is inserted into an XHTML document,
   *   htmlentities() must be called on it.
   */
  function url($id = NULL, $page = 0)
  {
    global $clean_urls;

    $url = '';
    if (!$clean_urls)
      $url .= 'process.php?s=';

    if (!$id)
      $id = $this->id;
    $url .= (int)$id;
    if ($clean_urls)
      $url .= '?';
    else
      $url .= '&';

    if ($page)
      $url .= 'page=' . (int)$page . '&';

    return $url;
  }

  /**
   * \brief
   *   A magic function which tries to upgrade old serialized sections
   *   to the new format.
   */
  function __wakeup()
  {
    if ($this->nclasses == -1)
      /* this Schedule doesn't need to be upgraded from Classes to Course */
      return;

    $this->courses = array();
    foreach ($this->classStorage as $classes)
      {
	$this->courses[] = $classes->to_course();
      }
    $this->nclasses = -1;
  }
}
