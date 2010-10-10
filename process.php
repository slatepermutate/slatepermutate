<?php

require_once('inc/schedule_store.inc');
require_once('inc/class.page.php');
include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';

// Converts a 5-element array into a nice string.
// Supports multiple modes, prettiness, and searching for different indicators
function arrayToDays($array, $mode = 'num', $pretty = false, $key = 1) {
	$outString = '';
	switch($mode){
		case 'short':
			$days = array('Mon','Tue','Wed','Thur','Fri');
			break;
		case 'long':
			$days = array('Monday','Tuesday','Wednesday','Thursday','Friday');
			break;
		case 'num':
			$days = array('1','2','3','4','5');
			break;
		default:
			$outString = 'Invalid mode passed to arrayToDays()!';
			return $outString;
	}
	if(count($array) > 1){
		for($i=0; $i<=4; $i++)	{
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
		for($i=0; $i<=4; $i++)
			if(isset($array[$i]))
				$outString = $days[$i];
	}
	return $outString;
}

function prettyTime($time){
	return substr($time,0,strlen($time)-2) . ":" . substr($time,strlen($time)-2, strlen($time));
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
	  $savedSched->writeoutTables();
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

	header('Location: input.php');
	exit;
      }
    elseif (!isset($_POST['postData']))
      {
	header('Location: input.php');
	exit;
      }
    else
      {
	/*
	 * we probably have input from the user and should interpret
	 * it as a schedule to permutate. Then we should redirect the
	 * user to the canonical URL for that schedule.
	 */
		$allClasses = new Schedule($_POST['postData']['name']);
	
		foreach($_POST['postData'] as $class)
		{
		  /*
		   * Only add classes if the user added at least one
		   * section to the class. We know that $class['name']
		   * is not a section, so count() needs to be > 1 and
		   * we need to skip over 'name' in our loop.
		   */
			if(is_array($class) && count($class) > 1)
			{
				$allClasses->addClass($class['name']);
		
				foreach($class as $section)
				  /* Skip the section name, which isn't a section */
					if(is_array($section))
					{
				             $allClasses->addSection($class['name'], $section['letter'], $section['start'], $section['end'], arrayToDays($section['days']));
					}
			}
		}
		$allClasses->findPossibilities();
		if (!isset($_SESSION['saved']))
		  $_SESSION['saved'] = array();
		$schedule_id = schedule_store_store($schedule_store, $allClasses);
		if ($schedule_id != NULL)
		  $_SESSION['saved'][$schedule_id] = $allClasses->getName();

		$process_php_s = '';
		if (!$clean_urls)
		  $process_php_s = 'process.php?s=';
		header('Location: ' . $process_php_s . $schedule_id);
		exit;
		/*
		 * writeoutTables() needs to know $schedule_id, so it
		 * has to be called after we save the schedule. See
		 * schedule_store_store().
		 */
		$allClasses->writeoutTables();
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
