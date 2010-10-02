<?php 

include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';
include_once 'inc/class.page.php';

$scripts = array('jQuery', 'jQueryUI', 'jValidate','schedInput');
$inputPage = new page('Scheduler', $scripts, FALSE);

$sch = FALSE;
if (isset($_REQUEST['savedkey']) && isset($_SESSION['saved']))
  {
    $savedkey = (int)$_REQUEST['savedkey'];
    if (isset($_SESSION['saved'][$savedkey]))
      {
	$sch = unserialize($_SESSION['saved'][$savedkey]);
      }
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
$inputPage->showSavedScheds($_SESSION);
?>
<p>Welcome to SlatePermutate! To get started, enter in some of your classes, and add available sections for each class.</p>
<form method="post" action="process.php" id="scheduleForm">
<table id="container">
  <tr><td>
    <table id="jsrows">
      <tr>
	<td colspan="11">
	  <input id="scheduleName" style="margin-bottom: 2em;" class="defText required" type="text" size="25" title="Schedule Name (e.g., Spring <?php echo Date('Y'); ?>)" name="postData[name]"
		 <?php if ($sch) echo 'value="' . str_replace('"', '&quot;', $sch->getName()) . '"'; /*"*/ ?>
		 />
	</td>
      </tr>
      <tr>
	<td class="advanced" colspan="11" style="padding-bottom: 2em;">
	  Section Labels are <select id="isNumeric" class="required" name="isnumbered">
	    <?php $isSelected = 'selected="selected"'; ?>
	    <option value="numerous" <?php if(!$sch || $sch->section_format == "numerous") echo $isSelected ?> >Custom</option>
	    <option value="numbered" <?php if($sch && $sch->section_format == "numbered") echo $isSelected ?> >Numbered</option>
	    <option value="lettered" <?php if($sch && $sch->section_format == "lettered") echo $isSelected ?> >Lettered</option>
	  </select>
	</td>
	</tr>
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

<!-- <div class="paddingtop" id="classage"><input type="button" value="Add class" /></div> -->
<div class="paddingtop"><input style="float:left;" type="submit" value="Find a schedule" /></div>

</form>

<p>&nbsp;<br /><br /><br /></p>
<p><span id="showadvanced" style="margin-left: 1em;"><a href="#">Advanced</a></span></p>
<?php
$inputPage->foot();
