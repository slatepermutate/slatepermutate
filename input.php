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

include_once 'inc' . DIRECTORY_SEPARATOR . 'class.schedule.php';
include_once 'inc' . DIRECTORY_SEPARATOR . 'class.course.inc';
include_once 'inc' . DIRECTORY_SEPARATOR . 'class.section.php';
include_once 'inc' . DIRECTORY_SEPARATOR . 'class.page.php';
require_once('inc' . DIRECTORY_SEPARATOR . 'schedule_store.inc');

$scripts = array('jQuery', 'jQueryUI', 'qTip','schedInput');
$inputPage = page::page_create('Scheduler', $scripts, FALSE);

$schedule_store = FALSE;
$sch = FALSE;
$school = $inputPage->get_school();

$parent_schedule_id = NULL;
if (isset($_REQUEST['s']))
  {
    $schedule_store = schedule_store_init();
    $parent_schedule_id = (int)$_REQUEST['s'];
    $sch = schedule_store_retrieve($schedule_store, $parent_schedule_id);
  }

$my_hc = 'var slate_permutate_example_course_id = \'' . str_replace('\'', '\\\'', school_example_course_id($inputPage->get_school())) . '\';

jQuery(document).ready(
  function()
  {
    var class_last = 0;

';
if ($sch)
{
  $nclasses = $sch->nclasses_get();
  for ($class_key = 0; $class_key < $nclasses; $class_key ++)
    {
      $my_hc .= input_class_js($sch->class_get($class_key), '    ');
    }
}
else
  {
    $default_courses = school_default_courses($school);
    foreach ($default_courses as $default_class)
      $my_hc .= input_class_js($default_class, '    ');
  }
$my_hc .= '    class_last = add_class();' . PHP_EOL;
if ($qtips_always || !isset($_SESSION['saw_qtips']))
  {
    $my_hc .= '    addTips();' . PHP_EOL;
    $_SESSION['saw_qtips'] = TRUE;
  }
$my_hc .= '  });
';

$inputPage->headcode_add('scheduleInput', $inputPage->script_wrap($my_hc), TRUE);

$inputPage->head();

/*
 * Force a student to choose a school or declare he's a generic
 * student before displaying the input form. To do this, we need
 * another variable in $_SESSION: $_SESSION['school_chosen'].
 */
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
  Choose the school you attend from the list below. <strong>If you cannot
  find your school</strong>, you may proceed using
  the <a href="<?php echo $next_page . (strpos($next_page, '?') === FALSE ? '?' : '&amp;'); ?>school=default">generic
  settings</a>.
</p>
<?php
    $inputPage->showSchools($next_page);
    $inputPage->foot();
    exit;
  }

if (!empty($_REQUEST['selectsemester']))
  {
?>
<h2>Semester Selection</h2>
<p>
  Choose the semester for which you wish you make a schedule from the
  list below. If any semester is missing, please <a
  href="feedback.php?feedback=My+school+is+missing+the+&lt;semester+name&gt;+semester.">let us know</a>.
</p>
<?php
  $inputPage->showSemesters();
  $inputPage->foot();
  exit;
  }

$inputPage->showSavedScheds($_SESSION);
?>
<p>
  Welcome to SlatePermutate<?php $inputPage->addressStudent(', ', '', FALSE); ?>!
  <?php if (school_has_auto($inputPage->get_school())): ?>
  To get started, enter in some a course identifier (e.g., <em>
  <?php echo school_example_course_id($inputPage->get_school()); ?></em>)
  and click the autosuggestion to automatically load available sections
  for each class.
  <?php else: ?>
  To get started, enter a course number and add some sections to it.
  Then specify each section's letter/number and what times it meets,
  add more courses, and click &ldquo;Find a Schedule&rdquo;.
  <!--'-->
  <?php endif; ?>
</p>

<form method="post" action="process.php" id="scheduleForm">
<p class="nospace" style="border-left: 5px solid #999; padding-left: 5px!important; padding-top: 5px!important;"><label>Schedule Name</label><br />
  <input id="scheduleName" style="margin-bottom: 1em;" class="defText required" type="text" size="25" title="<?php echo $inputPage->semester['name'] ?>" name="postData[name]" <?php if ($sch) echo 'value="' . htmlentities($sch->getName(), ENT_QUOTES) . '"'; ?> />
  <?php if (!empty($parent_schedule_id)): ?>
  <input type="hidden" name="postData[parent_schedule_id]" value="<?php echo htmlentities($parent_schedule_id); ?>" />
  <?php endif; ?>
</p>

<table id="container">
  <tr>
    <td>
      <table id="jsrows">
	<!-- Allow CSS to apply to entire rows at a time. -->
	<colgroup>
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col class="saturday<?php if (school_has_auto($inputPage->get_school())) echo ' collapsed';?>" />
	  <col />
	  <col />
	</colgroup>
        <!-- Header -->
        <tr>
          <td>Class ID</td>
          <td class="center" id="letterNumber">Section</td>
          <td class="center">Prof</td>
          <td class="center">Start Time</td>
          <td class="center">End Time</td>
          <td class="center">M</td>
          <td class="center">Tu</td>
          <td class="center">W</td>
          <td class="center">Th</td>
          <td class="center">F</td>
	  <td class="center">S</td>
          <td class="center"></td>
          <td class="center"></td>
        </tr>
      </table>
    </td>
  </tr>
</table>

<div class="paddingtop">
  <input class="button olive" type="submit" value="Find a schedule" />
</div>

</form>

<?php 

/* Show/hide Advanced Options: <p><span id="showadvanced" style="margin-left: 1em;"><a href="#">Advanced</a></span></p> */ 
?>
<div id="showInstructions" style="width: 100%; text-align: center;"><a href="#">Detailed Instructions...</a></div>

<?php
$inputPage->showSchoolInstructions();
$inputPage->foot();

function input_class_js(Course $class, $whitespace = '  ')
{
  $js = $whitespace . 'class_last = add_class_n(\'' . htmlentities($class->getName(), ENT_QUOTES) . "');\n";

  $nsections  = $class->getnsections();
  for ($section_key = $nsections - 1; $section_key >= 0; $section_key --)
    {
      $section = $class->getSection($section_key);
      $meetings = $section->getMeetings();
      foreach ($meetings as $meeting)
	{
	  $js .= $whitespace . 'add_section_n(class_last, \'' . htmlentities($section->getLetter(), ENT_QUOTES) . '\', \''
	    . htmlentities($section->getSynonym(), ENT_QUOTES) . '\', \''
	    . $meeting->getStartTime() . '\', \''
	    . $meeting->getEndTime() . '\', '
	    . json_encode(array('m' => $meeting->getDay(0), 't' => $meeting->getDay(1), 'w' => $meeting->getDay(2), 'h' => $meeting->getDay(3), 'f' => $meeting->getDay(4),
				's' => $meeting->getDay(5))) . ', \''
	    . htmlentities($section->getProf(), ENT_QUOTES) . '\', \''
	    . htmlentities($meeting->getLocation(), ENT_QUOTES) . '\',\''
	    . htmlentities($meeting->type_get(), ENT_QUOTES) . "');\n";
	}
    }

  return $js;
}
