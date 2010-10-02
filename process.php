<?php

session_start();

include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';

function sortInputs($post){
//	return array_filter($post['postData']); // Remove any null or unset items. Disabled as it kills day stuff, @FIXME and add day unset setting here (==0).
	return $post['postData'];
}


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

$DEBUG = false;
if(isset($_GET['debug']))
	$DEBUG = $_GET['debug'];

if(!$DEBUG){

	if(isset($_GET['savedkey'])){
		$savedSched = unserialize($_SESSION['saved'][$_GET['savedkey']]);
		$savedSched->writeoutTables();
	}
	else if(isset($_GET['delsaved'])){
		$_SESSION['saved'][$_GET['delsaved']] = '';
		$_SESSION['saved'] = array_filter($_SESSION['saved']); // Remove null entries
              header( 'Location: input.php' ) ;

	}
	else{
		$allClasses = new Schedule($_POST['postData']['name']);
	
		foreach(sortInputs($_POST) as $class)
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
		$allClasses->writeoutTables();
		if(!isset($_SESSION['saved']))
			$_SESSION['saved'] = array();
		array_push ( $_SESSION['saved'], serialize($allClasses));
	}
} else {


	echo '<pre>DEBUG OUTPUT: <br /><br />';
	foreach(sortInputs($_POST) as $class) {
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
