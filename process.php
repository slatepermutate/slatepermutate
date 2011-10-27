<?php /* -*- mode: php; -*- */
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

require_once('inc/schedule_store.inc');
require_once('inc/class.page.php');
include_once 'inc/class.schedule.php';
include_once('inc/class.course.inc');
include_once 'inc/class.section.php';

// Converts a 6-element day array into a string.
// Supports multiple modes, prettiness, and searching for different indicators
function arrayToDays($array, $mode = 'num', $pretty = false, $key = 1) {
	$outString = '';
	switch($mode)
	  {
		case 'short':
			$days = array('Mon', 'Tue', 'Wed', 'Thur', 'Fri', 'Sat');
			break;
		case 'long':
			$days = array('Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
			break;
		case 'num':
			$days = array('1','2','3','4','5');
			break;
	  case 'alpha':
	    $days = array('m', 't', 'w', 'h', 'f', 's');
	    break;
		default:
			$outString = 'Invalid mode passed to arrayToDays()!';
			return $outString;
	}
	if(count($array) > 1){
		for($i = 0; $i < 6; $i ++)
		{
			if(isset($array[$i]) && $array[$i] == $key){
				$outString .= $days[$i];
				if($pretty)
					$outString .= ', ';
			}
		}
		if($pretty){
			$outString = substr($outString,0,strlen($outString) - 2); // Remove last comma and space
			$outString = substr($outString,0, strrpos( $outString, ' ')) . ' and' . substr($outString, strrpos( $outString, ' '), strlen($outString));
		}
	}
	else {
		for($i=0; $i < 6; $i++)
			if(isset($array[$i]))
				$outString = $days[$i];
	}
	return $outString;
}

function prettyTime($time){
	return substr($time,0,strlen($time)-2) . ":" . substr($time,strlen($time)-2, strlen($time));
}

/**
 * \brief
 *   Convert a multidimensional array to a set of <input />s.
 *
 * Currently just echos out the <input />s as they are created.
 *
 * \param $array
 *   The array to make into a set of <input />s.
 * \param $base
 *   The string to prefix. Normally the name of the array variable.
 * \param $blankness
 *   A string to insert at the beginning of each line before an <input
 *   /> for indentation's sake.
 */
function array_to_form($array, $base = '',  $blankness = '        ')
{
  foreach ($array as $key => $val)
    if (is_array($val))
      array_to_form($val, $base . '[' . $key . ']', $blankness);
    else
      echo $blankness . '<input name="' . htmlentities($base . '[' . $key . ']') . '" value="' . htmlentities($val) . '" type="hidden" />' . PHP_EOL;
}

/*
 * The below code relies on sessions being started already.
 */
page::session_start();

$DEBUG = FALSE;
if (isset($_GET['debug']))
  $DEBUG = $_GET['debug'];

$schedule_store = schedule_store_init();

if(!$DEBUG)
  {
    $s = FALSE;
    if (isset($_GET['s']))
      $s = $_GET['s'];

    if($s !== FALSE)
      {
	$savedSched = schedule_store_retrieve($schedule_store, $s);
	if ($savedSched)
	  $savedSched->writeoutTables($schedule_store);
	else
	  page::show_404('Unable to find a saved schedule with an ID of ' . $s . '.');
      }
    elseif(isset($_GET['del']))
      {
	/* Allow the user to delete schedules that he has stored in his session */
	if ($_SESSION['saved'][(int)$_GET['del']])
	  {
	    /* user owns this schedule ID */
	    schedule_store_delete($schedule_store, (int)$_GET['del']);
	    unset($_SESSION['saved'][(int)$_GET['del']]);
	  }

	page::redirect('input.php');
	exit;
      }
    elseif (!isset($_POST['postData']))
      {
	page::redirect('input.php');
	exit;
      }
    else
      {
	/*
	 * we probably have input from the user and should interpret
	 * it as a schedule to permutate. Then we should redirect the
	 * user to the canonical URL for that schedule.
	 */
	$page_create_options = array();
	if (!empty($_POST['postData']))
	  $postData = $_POST['postData'];

	$name = '';
	if (!empty($postData['name']))
	  $name = $postData['name'];

	$parent_schedule_id = NULL;
	if (!empty($postData['parent_schedule_id']))
	  {
	    $parent_schedule_id = (int)$postData['parent_schedule_id'];
	    $parent_schedule = schedule_store_retrieve($schedule_store, $parent_schedule_id);
	    /* Detect bad parent_schedule reference. */
	    if (empty($parent_schedule))
	      $parent_schedule_id = NULL;
	  }

	$school = NULL;
	if (!empty($postData['school']))
	  {
	    /*
	     * This function returns NULL if it can't find the school_id
	     * so we're all good -- this is a type of error which is
	     * better to silently ignore ;-).
	     */
	    $school = school_load($postData['school']);
	    $page_create_options['school'] = $school;
	  }

	$semester = NULL;
	if (!empty($school) && !empty($postData['semester']))
	  {
	    $semesters = school_semesters($school);
	    if (!empty($semesters[$postData['semester']]))
	      {
		$semester = $semesters[$postData['semester']];
		$page_create_options['semester'] = $semester;
	      }
	  }

	$allClasses = new Schedule($name, $parent_schedule_id, $school, $semester);

	$errors = array();
	foreach($postData as $course)
	  {
	    /*
	     * Only add classes if the user added at least one
	     * section to the class. We know that $course['name']
	     * is not a section, so count() needs to be > 1 and
	     * we need to skip over 'name' in our loop.
	     */
	    if(is_array($course) && count($course) > 1)
	      {
		if (empty($course['title']))
		  $course['title'] = '';

		if (empty($course['credit_hours']))
		  $course['credit_hours'] = -1;
		$allClasses->addCourse($course['name'], $course['title']);

				foreach($course as $section)
				  /* Skip the section name, which isn't a section */
					if(is_array($section))
					  {
					    if (empty($section['slot']))
					      $section['slot'] = 'default';

					    $error_string = $allClasses->addSection($course['name'], $section['letter'], $section['start'], $section['end'], arrayToDays(empty($section['days']) ? array() : $section['days'], 'alpha'), $section['synonym'], $section['professor'], $section['location'], $section['type'], $section['slot'], $section['credit_hours']);
					    if ($error_string !== NULL)
					      $errors[] = $error_string;
					  }
			}
		}

		/*
		 * Tell the user that his input is erroneous and
		 * require him to fix it.
		 */
		if (count($errors))
		  {
		    $error_page = page::page_create('Process Schedule — Errors', array('qTip2'), $page_create_options);
		    $error_page->head();

		    echo '        <p>' . PHP_EOL
		      . '          You have the following errors in your input:' . PHP_EOL
		      . '        </p>' . PHP_EOL
		      . '        <ul>' . PHP_EOL;
		    foreach ($errors as $error)
		      echo '          <li>' . $error . '</li>' . PHP_EOL;
		    echo '        </ul>' . PHP_EOL
		      . '        <h3>Solving Errors</h3>' . PHP_EOL
		      . '        <ul>' . PHP_EOL
		      . '          <li>Most importantly, click the <em>Fix</em> button below to return to the schedule editing page to resolve these errors. Hitting your browser\'s <em>Back</em> button will cause your input to be lost.</li>' . PHP_EOL
		      . '          <li>Ensure that no section\'s start or end times are left blank. Any blank start or end times are shown as <tt>none</tt> in the above error output.</li>' . PHP_EOL
		      . '          <li>Ensure that a section\'s end time is later in the day than its start time.</li>' . PHP_EOL
		      . '          <li>If you are having trouble resolving these issues, please feel free to <a href="feedback.php">leave us feedback</a>. Be sure to describe your problem with as much detail as possible; otherwise we may only be able to make conjectures about the errors instead of finding and fixing any bugs. Thanks! <em>(To provide us with the most reliable data, save this webpage onto disk and paste the entire (X)HTML source into the feedback form.)</em></li>' . PHP_EOL
		      . '        </ul>' . PHP_EOL;

		    /* Regurgitate the postData into a <form /> */
		    echo '        <form action="input.php" method="post">' . PHP_EOL
		      . '          <input name="e" value="1" type="hidden" />' . PHP_EOL;
		    array_to_form($postData, 'postData', '          ');
		    echo '          <button type="submit" class="gray">Fix Errors!</button>' . PHP_EOL
		      . '        </form>' . PHP_EOL;

		    $error_page->foot();
		    exit;
		  }

		$allClasses->findPossibilities();
		if (!isset($_SESSION['saved']))
		  $_SESSION['saved'] = array();
		$schedule_id = schedule_store_store($schedule_store, $allClasses);
		if ($schedule_id != NULL)
		  $_SESSION['saved'][$schedule_id] = $allClasses->getName();

		page::redirect($allClasses->my_url());
		exit;
      }
  }
else
  {
	echo '<pre>DEBUG OUTPUT: <br /><br />';
	foreach($_POST['postData'] as $class) {
		echo 'Class: ' . $class['name'] . '<br />';
		foreach($class as $section)
			if(is_array($section))
			{
				echo '---- Section that starts at ' . prettyTime($section['start']) . ' and ends at ' . prettyTime($section['end']) . '. This class meets on ';
				echo arrayToDays($section['days'],'long',true) . '.<br />';
			}
		echo '<br />';
	}
	echo '</pre>';


}
