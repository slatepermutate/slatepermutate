<?php 

session_start();

include_once 'errors.php';
include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';
include_once 'inc/class.page.php';

$scripts = array('jquery','jValidate','schedInput');
$inputPage = new page('Scheduler', $scripts);
?>

<p>

<?
if(isset($_SESSION['saved']) && count($_SESSION['saved']) > 0){
	echo '<div id="savedBox" ><h3>Saved Schedules:</h3>';
	foreach($_SESSION['saved'] as $key => $schedule){
		$sch = unserialize($schedule);
		echo "<a href=\"process.php?savedkey=$key\">#" . ($key + 1) . " - " . $sch->getName() . "</a> <em><a href=\"process.php?delsaved=$key\"><img src=\"images/close.png\" style=\"border:0;\" /></a></em><br />";
	}
	echo '</div>';
}
?>

	</p>

<form method="post" action="process.php" id="scheduleForm">

<table id="jsrows">
	<tr>
		<td colspan="11" style="padding-bottom:2em;"><input id="scheduleName" class="defText" type="text" class="required" title="Schedule Name" name="postData[name]" />
			<em>(For example: Fall <?php echo Date("Y"); ?>)</em>
		</td>
	</tr>

<!-- Header -->
	<tr>
		<td>Class</td>
		<td class="center">Letter</td>
		<td class="center">Start Time</td>
		<td class="center">End Time</td>
		<td class="center">M</td>
		<td class="center">Tu</td>
		<td class="center">W</td>
		<td class="center">Th</td>
		<td class="center">F</td>
		<td class="center">Add</td>
		<td class="center">Delete</td>
	</tr>
</table>

<div class="paddingtop" id="classage"><input type="button" value="Add class" /></div>

<div class="paddingtop"><input style="float:left;" type="submit" value="Submit" /></div>

<p>
	<br />
</p>

<div class="paddingtop" id="reset"><input style="float:left;" type="button" value="Reset" /></div>

</form>

<p>
	&nbsp;
	<br />
</p>

<h3>TODO:</h3>

<ul>
	<li>Form validation to ensure endtime is after starttime, at least one day is checked.</li>
	<li>Check the saved schedule function. After input my default schedule, the output was the same
		as that of the demo. However, when I went back and clicked the "Saved Schedule 0" link, there 
		were then 48 possible schedules. It seems that the classes were added twice for some reason.</li>
	<li>Is there some way to automatically populate the fields when a user clicks on a saved schedule?
		This would probably be very useful, in the case that the individual would like to edit a saved
		schedule. Right now, it is only possible to edit the most previous schedule using the back button
		in the browser and this still requires the user to add the classes and schedules (although they
		already populated as soon as they are added).</li>
</ul>

<?
$inputPage->foot();
?>