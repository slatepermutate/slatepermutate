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
<table>
  <tr>
    <table id="jsrows">
	<tr>
		<td colspan="11" style="padding-bottom:2em;"><input id="scheduleName" class="defText" type="text" class="required" title="Schedule Name" name="postData[name]" />
			<em>(For example: Fall <?php echo Date("Y"); ?>)</em>
		</td>
	</tr>
	<tr>
		<td colspan="11" style="padding-bottom: 2em;">Section Numbering Style: <select id="isNumeric" type="text" class="required" name="isnumbered" ><option value="numerous">Custom</option><option value="numbered">Numbered</option><option value="lettered">Lettered</option></select>

	<!-- Header -->
	<tr>
		<td>Class</td>
		<td class="center" id="letterNumber">Section</td>
		<td class="center">Start Time</td>
		<td class="center">End Time</td>
		<td class="center">M</td>
		<td class="center">Tu</td>
		<td class="center">W</td>
		<td class="center">Th</td>
		<td class="center">F</td>
		<td class="center"></td>
		<td class="center"></td>
	</tr>
    </table>
  </tr>
  <tr><td> <span class="gray" style="padding: 0 3.5em 0 3.5em;" id="addclass">Add Class</span></td></tr>
</table>

<!-- <div class="paddingtop" id="classage"><input type="button" value="Add class" /></div> -->
<div class="paddingtop"><input style="float:left;" type="submit" value="Submit" /></div>

</form>

<p>&nbsp;<br /></p>

<h3>TODO:</h3>

<ul>
	<li>Add print stylesheet and button to output page</li>
	<li>After selecting a start time, set the end time to one hour after the start time</li>
        <li><strong>Append</strong> sections</li>
        <li>Move the add class button to somewhere nicer, maybe a gray row at the bottom. Make the submit button more obvious.</li>
	<li>Form validation to ensure endtime is after starttime, at least one day is checked.</li>
	<li>Auto-populate form based on saved schedule?</li>
        <li>Grab data from school sites such as <a href="http://www.cedarville.edu/courses/schedule/2010fa_be_bebl.htm" target="_blank">this?</a></li>
</ul>

<?php
$inputPage->foot();
