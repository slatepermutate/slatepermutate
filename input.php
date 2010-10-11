<?php 

include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';
include_once 'inc/class.page.php';
require_once('inc/schedule_store.inc');

$scripts = array('jQuery', 'jQueryUI', 'jValidate','schedInput');
$inputPage = new page('Scheduler', $scripts, FALSE);

$schedule_store = FALSE;
$sch = FALSE;
if (isset($_REQUEST['s']))
  {
    $schedule_store = schedule_store_init();
    $schedule_id = (int)$_REQUEST['s'];
    $sch = schedule_store_retrieve($schedule_store, $schedule_id);
  }

if ($sch)
{
  $nclasses = $sch->nclasses_get();
  $my_hc = '<script type="text/javascript">
var classNum = ' . $nclasses . ';
/* holds number of sections for each class */
var sectionsOfClass = new Array();
';
  for ($class_key = 0; $class_key < $nclasses; $class_key ++)
    $my_hc .= 'sectionsOfClass[' . $class_key . '] = ' . $sch->class_get($class_key)->getnsections() . ";\n";
  $my_hc .= '// </script>';
  $inputPage->headcode_add('scheduleInput', $my_hc, TRUE);
}
else
  $inputPage->headcode_add('schduleInput', '<script type="text/javascript">
var classNum = 0;
/* holds number of sections for each class */
var sectionsOfClass = Array();
// </script>', TRUE);

$inputPage->head();

/*
 * Force a student to choose a school or declare he's a generic
 * student before displaying the input form. To do this, we need
 * another variable in $_SESSION: $_SESSION['school_chosen'].
 */
$school = $inputPage->get_school();
if ($school && (!empty($_REQUEST['school']) || $school['id'] != 'default'))
  $_SESSION['school_chosen'] = TRUE;
if (!empty($_REQUEST['selectschool'])
    || $school['id'] == 'default' && !isset($_SESSION['school_chosen']))
  {
    $next_page = 'input.php';
    if (isset($_GET['s']))
      $next_page .= '?s=' . (int)$_GET['s'];
?>
<h2>School Selection</h2>
<p>
  Choose the school you attend from the list below. <b>If you cannot
  find your school</b>, you may proceed using
  the <a href="<?php echo $next_page . (strpos($next_page, '?') === FALSE ? '?' : '&amp;'); ?>school=default">generic
  settings</a>.
</p>
<?php
    $inputPage->showSchools($next_page);
    $inputPage->foot();
    exit;
  }

$inputPage->showSavedScheds($_SESSION);
?>
<p>
  Welcome to SlatePermutate<?php $inputPage->addressStudent(', ', '',
  FALSE); ?>! To get started, enter in some of your
  classes, and add available sections for each class.
</p>
<form method="post" action="process.php" id="scheduleForm">
<br />
<label>Schedule Name</label><br />
<input id="scheduleName" style="margin-bottom: 1em;" class="defText required" type="text" size="25" title="(e.g., Spring <?php echo Date('Y'); ?>)" name="postData[name]"
<?php if ($sch) echo 'value="' . str_replace('"', '&quot;', $sch->getName()) . '"'; /*"*/ ?>
/>

<table id="container">
  <tr><td>
    <table id="jsrows">
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
	<?php if ($sch) echo $sch->input_form_render(); ?>
    </table>
  </td>
  </tr>
  
  <tr><td> <span class="gray" style="padding: 0 3.5em 0 3.5em;" id="addclass">Add Class</span></td></tr>
</table>

<div class="paddingtop"><input class="green" style="margin:0;padding:0;" type="submit" value="Find a schedule" /></div>

</form>

<p>&nbsp;<br /><br /><br /></p>
<?php /* RE-enable if advanced options added: <p><span id="showadvanced" style="margin-left: 1em;"><a href="#">Advanced</a></span></p> */ ?>
<?php
$inputPage->showSchoolInstructions();
$inputPage->foot();
