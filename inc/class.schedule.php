<?php /* -*- mode: php; indent-tabs-mode: nil; -*- */
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

$incdir = dirname(__FILE__) . DIRECTORY_SEPARATOR;
include_once $incdir . 'class.course.inc';
include_once $incdir . 'class.section.php';
include_once $incdir . 'class.page.php';
require_once $incdir . 'school.inc';
require_once $incdir . 'math.inc';

/*
 * Load a Classes -> Course converter class for the sake of the
 * Schedule::__wakeup() magic function.
 */
require_once $incdir . 'class.classes_convert.inc';

/**
 * \brief
 *   Finds possible Section combinations for a user's given Courses
 *   and stores and displays the results.
 *
 * Represents a schedule of a week. Stores the classes that are part
 * of that week and calculates all the possible permutations.
 */
class Schedule
{
  /*
   * Variables for upgrading from saved schedules created when there
   * was a class called Classes.
   */
  private $classStorage;			// array of courses
  private $nclasses;				// Integer number of classes

  /**
   * \brief
   *   Provides a mapping to regain the user's original input.
   *
   * Currently, the Schedule object cannot natively handle CourseSlot
   * objects properly. It assumes that each Course has one and only
   * one CourseSlot. This array maps each Course object stored in
   * $classStorage onto the index of the course it was originally
   * from. I.e., if the Course at index 0 had two CourseSlot objects,
   * array(0 => 0, 1 => 0, 2 => 1) would map these two CourseSlot
   * objects onto the same Course object and have the next CourseSlot
   * be mapped into a separate Course object.
   */
  private $course_slot_mappings;

  /* My member variables. */
  private $courses;
  private $nPermutations = 0;		// Integer number of real permutations
  private $possiblePermutations;	// Integer number of possible permutations
  private $scheduleName;			// String name of schedule
  private $storage;				// Integer array of valid schedules
  /**
   * \brief
   *   The school_id of the school this schedule was created for.
   */
  private $school_id;
  /**
   * \brief
   *   The semester this schedule was created for.
   *
   * The semester array is stored in full in a schedule because some
   * schools do not keep a backlog of all semesters for their course
   * data. Such a school is calvin. We want to be able to access, for
   * example, the friendly name and so on of a semester even one year
   * after the semester is created without worrying about having that
   * semester be stored in the autocomplete cache.
   */
  private $semester;

  /* The <title /> of the page used when rendering this schedule */
  private $title;

  /**
   * \brief
   *   My global identification number. Not defined until the schedule
   *   is processed and first saved.
   */
  private $id;

  /*
   * The identifier of the schedule from which this schedule was
   * derived or NULL.
   */
  private $parent_id;

  /**
   * \brief
   *   When I was created, a unix timestamp.
   */
  private $created;

  /**
   * \brief
   *   Create a schedule with the given name.
   *
   * \param $name
   *   A string, the friendly name the user gave this schedule.
   * \param $parent
   *   An integer, the id of the schedule from which this schedule is
   *   derived. A schedule is considered to be derived of another of
   *   the user created this schedule by clicking ``Edit'' for the
   *   previous schedule. Or NULL if this schedule stands on its own.
   * \param $school
   *   The school used for this schedule. The intention of storing
   *   this data is that people from different schools may share
   *   schedules with eachother. Also, people who bookmark their
   *   schedules and want to edit their schedules should not have to
   *   go through the school selection dialogue again but should just
   *   be set to use the correct school.
   * \param $semester
   *   The semester used for this schedule.
   */
  function __construct($name, $parent = NULL, array $school = NULL, array $semester = NULL)
  {
    $this->courses = array();
    $this->course_slot_mappings = array();
    $this->scheduleName = $name;
    $this->storage = array();
    $this->title = "SlatePermutate - Scheduler";
    $this->parent_id = $parent;

    if (empty($school))
      $school = school_load_guess(FALSE);
    $this->school_id = $school['id'];

    if (empty($semester))
      {
	$semester = school_semester_guess($school);
      }
    $this->semester = $semester;

    $this->created = time();

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

  /**
   * \brief
   *   Adds a new class to the schedule.
   *
   * \param $slot
   *   Currently, the Schedule class is not smart enough to understand
   *   CourseSlots. At a lower level, we split Courses with multiple
   *   CourseSlots into multiple Course objects with redundant
   *   information.
   */
  function addCourse($course_id, $title)
  {
    $this->courses[] = new Course($course_id, $title);
  }

  /**
   * \brief
   *   Adds a section to this semester after finding the class.
   *
   * \param $instructor
   *   The instructor of this section/section_meeting.
   *
   * \return
   *   NULL on success, a string on error which is a message for the
   *   user and a valid XHTML fragment.
   */
  function addSection($course_name, $letter, $time_start, $time_end, $days, $synonym = NULL, $instructor = NULL, $location = NULL, $type = 'lecture', $slot = 'default', $credit_hours = -1.0)
  {
    if (empty($letter) && (empty($time_start) || !strcmp($time_start, 'none')) && (empty($time_end) || !strcmp($time_end, 'none')) && empty($days)
	&& empty($synonym) && empty($instructor) && empty($location) && (empty($type) || !strcmp($type, 'lecture'))
	&& (empty($slot) || !strcmp($slot, 'default')))
      return;

    /* reject invalid times */
    if (!strcmp($time_start, 'none') || !strcmp($time_end, 'none')
	|| $time_start > $time_end)
      {
	return 'Invalid time specifications for ' . htmlentities($course_name) . '-' . htmlentities($letter)
	  . '. Start time: <tt>' . htmlentities($time_start) . '</tt>. End time: <tt>' . htmlentities($time_end) . '</tt>.';
      }

    if (!empty($credit_hours) && !is_numeric($credit_hours))
      {
        return 'Invalid credit-hour specification of <tt>' . htmlentities($credit_hours) . '</tt> for ' . htmlentities($course_name) . '-' . htmlentities($letter) . '. Please use a floating point number or do not enter anything if the number of credit hours is not known.';
      }

    foreach ($this->courses as $course)
      if (!strcmp($course_name, $course->getName()))
	{
	  $section = $course->section_get($letter);
	  if (!$section)
	    {
              $section = new Section($letter, array(), $synonym, $credit_hours);
	      $course->section_add($section, $slot);
	    }
	  $section->meeting_add(new SectionMeeting($days, $time_start, $time_end, $location, $type, $instructor));

	  return;
	}

    error_log('Could not find class when parsing schedule from postData: ' . $course_name);
    echo 'Could not find class: ' . $course_name . "<br />\n";
  }

  /**
   * \brief
   *   Get the school associated with this schedule.
   *
   * \return
   *   The school associated with this schedule or some fallback.
   */
  public function school_get()
  {
    $school = NULL;

    if (!empty($this->school_id))
      /*
       * May return NULL, so we don't just return this value right
       * away -- we fall through.
       */
      $school = school_load($this->school_id);
    if (empty($school))
      {
	/* Ensure we have $_SESSION. */
	page::session_start();
	$school = school_load_guess(FALSE);
      }

    return $school;
  }

  /**
   * \brief
   *   Get the semester associated with this schedule.
   *
   * \return
   *   The schedule's associated semester.
   */
  public function semester_get()
  {
    return $this->semester;
  }

  //--------------------------------------------------
  // Finds all of the possible permutations and stores
  // the results in the storage array.
  //--------------------------------------------------
	function findPossibilities()
	{
	  /*
	   * Split out any Course objects with multiple CourseSlots
	   * into multiple Course objects...
	   */
	  $new_courses = array();
	  foreach ($this->courses as $i => $course)
	    foreach ($course as $course_slot)
	      {
		$new_course = new Course($course->getName(), $course->title_get());
		$new_course->course_slot_add($course_slot);
		$new_courses[] = $new_course;

		$this->course_slot_mappings[count($new_courses) - 1] = $i;
	      }
	  $this->courses = $new_courses;
	  unset($new_courses);

	  /*
	   * Clean crud (completely empty courses) out of the
	   * schedule. For some crud, it's much easier to detect that
	   * it's crud now than during parsing of postData[].
	   *
	   * Now we may assume that each Course only has one
	   * CourseSlot...
	   */
	  foreach ($this->courses as $i => $course)
	    {
	      $course_slot = NULL;
	      foreach ($course as $course_slot)
		break;
	      if (empty($course_slot) || !$course_slot->sections_count())
		{
		  unset($this->courses[$i]);
		  $this->courses = array_values($this->courses);
		  return $this->findPossibilities();
		}
	    }

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
		  /*
		   * Kludge for until we support course_slots natively
		   * or find a better solution.
		   */
		  unset($course_slot);
		  foreach ($course as $course_slot)
		    break;

			$this->possiblePermutations = $this->possiblePermutations * $course_slot->sections_count();
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
				  unset($course_slot_up);
				  foreach ($this->courses[$upCounter] as $course_slot_up)
				    break;

				  unset($course_slot_down);
				  foreach ($this->courses[$downCounter] as $course_slot_down)
				    break;

				  if ($course_slot_up->section_get_i($cs[$upCounter])->conflictsWith($course_slot_down->section_get_i($cs[$downCounter])))
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
	    unset($course_slot);
	    foreach ($this->courses[$position] as $course_slot)
	      break;

	    if($cs[$position] == $course_slot->sections_count())
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

  /**
   * \brief
   *   Prints out the possible permutations in tables.
   *
   * \param $schedule_store
   *   The schedule_store handle with which this schedule was loaded,
   *   used to query the parent schedule.
   */
  //--------------------------------------------------
  function writeoutTables(array $schedule_store = NULL)
  {
    $filled = false;
    $time = array(700,730,800,830,900,930,1000,1030,1100,1130,1200,1230,1300,1330,1400,1430,1500,1530,1600,1630,1700,1730,1800,1830,1900,1930,2000,2030,2100,2130, 2200);

    define('SP_PERMUTATIONS_PER_PAGE', 64); /** @TODO: Define this in config.inc */

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

    $headcode = array('jQuery', 'jQueryUI', 'uiTabsKeyboard', 'displayTables', 'outputStyle', 'jQuery.cuteTime', 'qTip2');
    if(!empty($_REQUEST['print']))
      array_push($headcode, 'outputPrintStyle');
    else
      array_push($headcode, 'jAddress');

    $outputPage = page::page_create(htmlentities($this->getName()), $headcode,
				    array('school' => $this->school_get(), 'semester' => $this->semester_get()));
    $outputPage->head();



    if(!empty($_REQUEST['print']))
      {
	$script = ''
	  . 'jQuery(document).ready( function() {' . PHP_EOL
	  . '  window.print();' . PHP_EOL
	  . '});';
	echo $outputPage->script_wrap($script);
	echo '<p><a href="'.$_SERVER['SCRIPT_NAME'].'?s=' . $this->id_get() . '">&laquo; Return to normal view</a> </p>';
    }
    else {
      echo '        <script type="text/javascript">';
      echo '  jQuery(document).ready( function() {';
      echo '    jQuery("#tabs").tabs();';
      echo '    jQuery.address.change(function(event){';
      echo '      jQuery("#tabs").tabs( "select" , window.location.hash );';
      echo '    });';
      echo '    jQuery("#tabs").bind("tabsselect", function(event, ui) {';
      echo '      window.location.hash = ui.tab.hash;';
      echo '    });';


      echo '    jQuery("#sharedialog").dialog({ modal: true, width: 550, resizable: false, draggable: false, autoOpen: false });';
      echo '    jQuery("#share").click( function() {
                  jQuery("#sharedialog").dialog("open");
                });';
      echo '    jQuery(\'#printItems\').click( function() {
		  window.location = "'.$_SERVER['SCRIPT_NAME'].'?s='.$this->id_get().'&amp;print=" + (jQuery(\'#tabs\').tabs(\'option\',\'selected\') + 1);
	        });
	        jQuery(\'#cancelItems\').click( function() {
		  jQuery(\'#selectItemsInput\').hide();
	        });'
	. '    ' . PHP_EOL
	. '    jQuery(\'.cute-time\').cuteTime();' . PHP_EOL
	. '  });' . PHP_EOL
	. '        </script>' . PHP_EOL;

      echo '        <div id="sharedialog" title="Share Schedule">' . PHP_EOL
	. '          <p class="indent"><img alt="[fb]" class="noborder" src="http://facebook.com/favicon.ico" /> <a href="http://www.facebook.com/sharer.php?u=' . urlencode(htmlentities($outputPage->gen_share_url($this->id_get()))) .'&amp;t=My%20Schedule">Share on Facebook</a></p>
		     <p class="indent"><img alt="[sp]" class="noborder" src="images/favicon.png" style="margin-right: 5px;"/><span class="clicktoclipboard"><a href="#">Share with URL</a><span class="toclipboard hidden"><p>Copy the share URL below:<br /><em class="centeredtext smallurl">' . htmlentities($outputPage->gen_share_url($this->id_get())) . '</em></p></span></span></p>' . PHP_EOL
	. '        </div>' . PHP_EOL
	. '        <p>' . PHP_EOL
	. '          <a href="input.php?s='.$this->id.'" class="button">Edit</a>' . PHP_EOL
	. '          <span id="printItems"><a href="#" class="button">Print</a></span>' . PHP_EOL
	. '          <span id="share"><a href="#" class="button">Share</a></span>' . PHP_EOL;


      if ($schedule_store !== NULL
	  && $this->parent_get() !== NULL
	  && ($parent_schedule = schedule_store_retrieve($schedule_store, $this->parent_get())) !== NULL)
	echo '          <a class="button" href="' . htmlentities($parent_schedule->my_url()) . '" title="Parent schedule: ' . htmlentities($parent_schedule->getName()) . '">Parent</a>' . PHP_EOL;

      echo '          <a class="button" href="input.php">Home</a>' . PHP_EOL
	. '        </p>'. PHP_EOL
	. '        <p class="centeredtext">Having problems? <a href="feedback.php">Let us know</a>.</p>' . PHP_EOL
	. '        <p class="centeredtext graytext"><em>Keyboard Shortcut: Left and right arrow keys switch between schedules</em></p>' . PHP_EOL;
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
	  foreach ($course as $course_slot)
	  {
	    for ($si = 0; $si < $course_slot->sections_count(); $si ++)
              foreach ($course_slot->section_get_i($si)->getMeetings() as $meeting)
		{
		  /* Saturdayness */
		  if ($meeting->getDay(5))
		    {
		      $max_day_plusone = 6;
		      $have_saturday = TRUE;
		    }

		  /* very late / very early classes */
		  while ((int)ltrim($meeting->getEndTime(), '0') > $max_time)
		    {
		      $max_time += $max_time + 30;
		      while ($max_time % 100 >= 60)
			$max_time += 40; /* + 100 - 60 */
		      $time[] = $max_time;
		    }
		  while ((int)ltrim($meeting->getStartTime(), '0') < $min_time)
		    {
		      $max_time += 30;
		      while ($min_time % 100 < 30)
			$min_time -= 40; /* + 60 - 100 */
		      $min_time -= 30;
		      $time[] = $min_time;
		      $sort_time = TRUE;
		    }
		}
	  }
	/* ensure that early times are actually first ;-) */
	if ($sort_time)
	  sort($time);

        echo '    <div id="regDialog" title="Registration Codes">' . PHP_EOL
	  . '      <div id="regDialog-content"></div>' . PHP_EOL
	  . '      <p class="regDialog-disclaimer graytext">' . PHP_EOL
	  . '        <em>' . PHP_EOL
	  . '          Note: The registration information above corresponds to the sections' . PHP_EOL
	  . '          displayed on the currently selected tab.' . PHP_EOL
	  . '        </em>' . PHP_EOL
	  . '      </p>' . PHP_EOL
	  . '      <p class="regDialog-disclaimer graytext">' . PHP_EOL
	  . '        <em>' . PHP_EOL
	  . '          Disclaimer: You are responsible for' . PHP_EOL
	  . '          double-checking the information you get from and input into slate_permutate' . PHP_EOL
	  . '          when registering for classes. There is no guarantee that the harvested' . PHP_EOL
	  . '          information is correct or that slate_permutate will handle' . PHP_EOL
	  . '          the information you enter correctly.' . PHP_EOL
	  . '        </em>' . PHP_EOL
	  . '      </p>' . PHP_EOL
	  . '    </div>' . PHP_EOL;
	echo '<div id="tabs">' . "\n" .
               '<div id="show-box" class="show-buttons">
                  <form action="#"><p class="nospace">
                    <label><strong>Display:</strong></label>
                    <input id="show-course-title" name="show-course-title" type="checkbox" /><label for="show-course-title">Course Title</label>
                    <input id="show-prof" name="show-prof" type="checkbox" checked="checked" /><label for="show-prof">Professor</label>
                    <input id="show-location" name="show-location" type="checkbox" /><label for="show-location">Room</label>
                    <input id="show-synonym" name="show-synonym" type="checkbox" /><label for="show-synonym">Synonym</label>
                    <input id="show-credit-hours" name="show-credit-hours" type="checkbox" /><label for="show-credit-hours">Credits</label>
                    <span id="regCodes"><label><a href="#"><strong>Register for Classes</strong></a></label></span></p>
                  </form>
                </div> <!-- id="show-box" -->'
	     . '<div id="the-tabs"><ul>' . "\n";

	$suppressed_permutations = array();
	if (!empty($_REQUEST['print']))
	  {
	    $print = $_REQUEST['print'];
	    if ($print !== 'all')
	      {
		for ($i = $first_permutation; $i <= $last_permutation; $i ++)
		  $suppressed_permutations[$i] = TRUE;
		foreach (explode(',', $print) as $item_to_print)
		  unset($suppressed_permutations[((int)$item_to_print) - 1]);
	      }
	  }

	for($nn = $first_permutation + 1; $nn <= $last_permutation; $nn++)
	  {
	    if (!empty($suppressed_permutations[$nn - 1]))
	      continue;
	    echo  "<li><a href=\"#tabs-" . $nn . "\">&nbsp;" . $nn . "&nbsp;</a></li>\n";
	  }
			
	echo "    </ul></div>\n  \n";

	echo "    <div id=\"pagers\">\n";
	/* Previous button */
	if ($page > 0)
	  echo '      <div id="pager-previous" class="pager left"><a href="' . htmlentities($this->my_url($page - 1)) . '">&laquo; Previous</a></div>' . "\n";

	/* Next button */
	if ($page + 1 < $npages)
	  echo '      <div id="pager-next" class="pager right"><a href="' . htmlentities($this->my_url($page + 1)) . '">Next &raquo;</a></div>' . "\n";
	echo "    </div> <!-- id=\"pagers\" -->\n";


	echo "  <div class=\"scroller\">\n"
	  . "    <div class=\"scontent\">\n";
		
	for($i = $first_permutation; $i < $last_permutation; $i++)
	  {
	    /*
	     * Skip suppressed permutations, such as when displaying a
	     * page for printing a particular permutation.
	     */
	    if (!empty($suppressed_permutations[$i]))
	      continue;

	    /*
	     * Store a JSON list of courses, each with only the one
	     * section rendered in this permutation. This is used for
	     * the ``Registration Numbers'' dialog which noramlly
	     * shows users course synonyms.
	     */
	    $permutation_courses = array();

            /*
             * Count the number of credit hours in this particular
             * schedule.
             */
            $credit_hours = array();
            $have_credit_hours = FALSE;

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
			  $course = $this->courses[$j];
			  foreach ($course as $course_slot)
			    {
			      $section_index = $this->storage[$i][$j];
			      $section = $course_slot->section_get_i($section_index);

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

				      $title = $course->title_get();
				      if (empty($title))
					$title = '';
				      else
					$title .= ' ';

				      $carret = '&#013;' . htmlentities("<br />");
				      echo '            <td rowspan="' . $rowspan[$dayLoop]
					. '" class="qTipCell ' . $single_multi . ' class' . $j
					. '" title="' . htmlentities($title, ENT_QUOTES) . $carret
					. 'Prof: ' . htmlentities($current_meeting->instructor_get(), ENT_QUOTES) . $carret
					. 'Room: ' . htmlentities($current_meeting->getLocation(), ENT_QUOTES) . $carret
                                        . 'Type: ' . htmlentities($current_meeting->type_get(), ENT_QUOTES) . $carret;

                                      $section_credit_hours = $section->credit_hours_get();
                                      if ($section_credit_hours >= 0)
                                        {
                                          $credit_hours[$section->getSynonym()] = $section_credit_hours;
                                          $have_credit_hours = TRUE;

                                          echo 'Credits: ' . htmlentities($section_credit_hours, ENT_QUOTES) . $carret;
                                        }
                                      echo '">'
					. '<span class="course-title block">' . htmlentities($title) . '</span>' . PHP_EOL
					. htmlentities($course->getName(), ENT_QUOTES) . '-'
					. htmlentities($section->getLetter(), ENT_QUOTES) . "\n"
					. '<span class="prof block">' . htmlentities($current_meeting->instructor_get(), ENT_QUOTES) . "</span>\n"
					. '<span class="location block">' . htmlentities($current_meeting->getLocation(), ENT_QUOTES) . "</span>\n"
					. '<span class="synonym block">' . htmlentities($section->getSynonym(), ENT_QUOTES) . "</span>\n"
                                        . '<span class="credit-hours block">' . htmlentities($section_credit_hours, ENT_QUOTES) . ' Credits</span>' . PHP_EOL
					. "</td>\n";

				      /* for the ``Registration Codes'' dialogue: */
				      if (empty($permutations_courses[$j]))
					{
					  $singleton_course = new Course($course->getName(), $course->title_get());
					  $singleton_course->section_add($section, $course_slot->id_get());
					  $permutation_courses[$j] = $singleton_course->to_json_array();
					}

				      $filled = TRUE;
				    }
			    } /* $course_slot */
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
	    echo "        </table>\n";

            if ($have_credit_hours)
              echo '        <p>Credit Hours: ' . sum($credit_hours) . '</p>' . PHP_EOL;

            echo ''
              . '        <span class="course-data">'.  htmlentities(json_encode($permutation_courses)) . "</span>\n"
	      . '      </div> <!-- id="section' . ($i + 1) . "\" -->\n";
	  }

          echo "    </div> <!-- class=\"scontent\" -->\n"
	     . "  </div> <!-- class=\"scroller\" -->\n"
	     . "</div> <!-- id=\"my-glider\" -->\n"
	     . $footcloser; // Closes off the content div
      } else {
      echo '<html><body><p>There are no possible schedules. Please <a href="input.php?s='.$this->id.'">try again</a>.</p></body></html>';
    }

    echo '<p id="possiblestats">' . PHP_EOL
      . '  There were a total of ' . $this->possiblePermutations . ' possible permutations. Only ' . $this->nPermutations . ' permutations had no class conflicts.' . PHP_EOL
      . '</p>' . PHP_EOL;
    if ($this->created)
      echo ''
	. '<p id="created-time">' . PHP_EOL
	. '  Created <span class="cute-time">' . gmdate('c', $this->created) . '</span>.' . PHP_EOL
	. '</p>' . PHP_EOL;

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
    if($t >= 1200)
      {
	if ($t > 1259)
	  $t = ($t - 1200);
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
   *   Fetch a specified class by its key.
   *
   * Use Schedule::courses_get() instead of this function if the code
   * you're writing understand CourseSlot objects.
   *
   * \see Schedule::courses_get().
   */
  public function class_get($class_key)
  {
    return $this->courses[$class_key];
  }

  /**
   * \brief
   *   Get an array of Course objects as originally inputted by the
   *   user.
   */
  public function courses_get()
  {
    /*
     * As Mr. Westra would say, just map them courses back into their
     * original forms.
     */

    $courses = array();
    foreach ($this->courses as $course_i => $course)
      {
	$mapping = $this->course_slot_mappings[$course_i];
	if (empty($courses[$mapping]))
	  $courses[$mapping] = new Course($course->getName(), $course->title_get());

	foreach ($course as $course_slot)
	  $courses[$mapping]->course_slot_add($course_slot);
      }

    return $courses;
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
   *   The ID of the schedule to link to.
   * \param $page
   *   The page/tab of the schedule to link to. Defaults to 0.
   * \return
   *   A string, the URL used to access the specified
   *   schedule. Remember that if this string is inserted into an
   *   XHTML document, htmlentities() must be called on it.
   */
  public static function url($id, $page = 0)
  {
    global $clean_urls;

    $url = '';
    if (!$clean_urls)
      $url .= 'process.php?s=';

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
   *   Return the URL used to access this schedule.
   *
   * Convenience wrapper around Schedule::url().
   *
   * \param $page
   *   Which page (tab) of the schedule to link to.
   * \return
   *   A raw URL (one that must be htmlentities()ized before putting
   *   into HTML).
   */
  public function my_url($page = 0)
  {
    return Schedule::url($this->id, $page);
  }

  /**
   * \brief
   *   Get the ID of the schedule from which this schedule was
   *   derived.
   */
  public function parent_get()
  {
    return $this->parent_id;
  }

  /**
   * \brief
   *   Get the unix timestamp of when this schedule was created
   *   (saved).
   *
   * \return
   *   A unix timestamp. 0 if the timestamp is unavailable.
   */
  public function created_get()
  {
    return $this->created;
  }

  /**
   * \brief
   *   A magic function which tries to upgrade old serialized sections
   *   to the new format.
   */
  function __wakeup()
  {
    if ($this->nclasses != -1)
      {
	/* this Schedule needs to be upgraded from Classes to Course */

	$this->courses = array();
	foreach ($this->classStorage as $classes)
	  $this->courses[] = $classes->to_course();
	$this->nclasses = -1;
      }

    if (empty($this->parent_id))
      $this->parent_id = NULL;

    if (empty($this->school_id))
      {
	/* Ensure we have $_SESSION. */
	page::session_start();
	$school = school_load_guess(FALSE);
	$this->school_id = $school['id'];
      }
    if (empty($this->semester))
      {
	if (empty($school))
	  {
	    /* Ensure we have $_SESSION. */
	    page::session_start();

	    $school = school_load($this->school_id);
	    $this->semester = school_semester_guess($school);
	  }
      }

    if (empty($this->course_slot_mappings))
      {
	$this->course_slot_mappings = array();
	foreach ($this->courses as $course_i => $course)
	  $this->course_slot_mappings[$course_i] = count($this->course_slot_mappings);
      }

    if (empty($this->created))
      $this->created = 0;
  }
}
