<?php 

include_once 'errors.php';
include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';
include_once 'inc/class.page.php';

$scripts = array('jQuery','jValidate','schedInput');
$inputPage = new page('Scheduler', $scripts);
$inputPage->showSavedScheds($_SESSION);

?>

<form method="post" action="process.php" id="scheduleForm">
<table id="jsrows">
	<tr>
		<td colspan="11" style="padding-bottom:2em;"><input id="scheduleName" class="defText" type="text" class="required" title="Schedule Name" name="postData[name]" />
			<em>(For example: Fall <?php echo Date("Y"); ?>)</em>
		</td>
	</tr>
	<tr>
		<td colspan="11" style="padding-bottom: 2em;">Sections at my school are <select id="isNumeric" type="text" class="required" name="isnumbered" ><option value="numbered">Numbered</option><option value="lettered">Lettered</option></select>

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

</form>

<p>&nbsp;<br /></p>

<h3>TODO:</h3>

<ul>
	<li>Form validation to ensure endtime is after starttime, at least one day is checked.</li>
	<li>Auto-populate form based on saved schedule?</li>
</ul>

<? $inputPage->foot(); ?>
