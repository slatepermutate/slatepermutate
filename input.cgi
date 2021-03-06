#!/usr/bin/env php-cgi
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

/*
 * Help constrol whether or not the school selection dialogue should
 * be shown or whether or not $_SESSION['school_chosen'] should be set
 * TRUE. These things should generally be false when loading a saved
 * schedule.
 */
$creating_new_schedule = TRUE;

$schedule_store = FALSE;
$sch = FALSE;
$errors_fix = FALSE;
$inputPage_options = array('school_semester_constant' => FALSE);

$parent_schedule_id = NULL;
if (isset($_REQUEST['s']))
  {
    $schedule_store = schedule_store_init();
    $parent_schedule_id = (int)$_REQUEST['s'];
    $sch = schedule_store_retrieve($schedule_store, $parent_schedule_id);

    /*
     * Allow a user to change the school and semester of a
     * saved_schedule he's trying to revive if he really wants to.
     */
    if (!empty($_GET['school']))
      $school = school_load_guess(FALSE);
    else
      $school = $sch->school_get();

    if (!empty($_GET['semester']))
      $semester = school_semester_guess($school, FALSE);
    else
      $semester = $sch->semester_get();

    if (!empty($sch))
      {
	$creating_new_schedule = FALSE;
	$inputPage_options += array('school' => $school,
				    'semester' => $semester);
      }
    else
      $parent_schedule_id = NULL;

    /*
     * Code outside of this block should _not_ assume $school and/or
     * $semester are defined. But it'd be more expensive to unset()
     * them here than to just overwrite them later...
     */
  }
elseif (!empty($_REQUEST['e']))
  {
    /*
     * Read an errorful schedule out of $_POST, this $_POST is created
     * by process.cgi when the originally sinful user produces bad
     * data.
     */
    $errors_fix = TRUE;

    if (!empty($_POST['postData']))
      $postData = $_POST['postData'];

    if (!empty($postData['parent_schedule_id']))
      $parent_schedule_id = (int)$postData['parent_schedule_id'];

    if (!empty($postData['school']))
      {
	$school = school_load($postData['school']);
	if (!empty($school))
	  $inputPage_options['school'] = $school;
      }

    if (!empty($school) && !empty($postData['semester']))
      {
	$semesters = school_semesters($school);
	if (!empty($semesters[$postData['semester']]))
	  $inputPage_options['semester'] = $semester;
      }

    $creating_new_schedule = FALSE;
  }

/*
 * We cannot initialize the page object nor guess the school before
 * figuring loading a saved schedule because we'll default to that
 * saved_schedule's school/semester.
 */
$scripts = array('jQuery', 'jQueryUI', 'qTip2', 'schedInput');
$inputPage = page::page_create('Enter Courses', $scripts, $inputPage_options);
$school = $inputPage->get_school();
$semester = $inputPage->semester_get();
if (empty($semesters))
  $semesters = school_semesters($school);

$my_hc = 'var slate_permutate_example_course_id = ' . json_encode(empty($semester) || empty($semester['popular_course_id']) ? school_example_course_id($school) : $semester['popular_course_id']) . ';

jQuery(document).ready(
  function()
  {
    var class_last = 0;

';
if ($sch)
{
  foreach ($sch->courses_get() as $course)
    {
      $my_hc .= input_course_js($course, '    ');
    }
}
elseif ($errors_fix)
  {
    foreach ($_POST['postData'] as $course)
      if (is_array($course))
	{
	  $title = '';
	  if (!empty($course['title']))
	    $title = $course['title'];
	  if (empty($course['name']))
	    $my_hc .= '    class_last = add_class();' . PHP_EOL;
	  else
	    $my_hc .= '    class_last = add_class_n(' . json_encode($course['name']) . ', ' . json_encode($title) . ');' . PHP_EOL;
	  foreach ($course as $section)
	    if (is_array($section))
	      $my_hc .= '    add_section_n(class_last, ' . json_encode($section['letter']) . ', '
		. json_encode($section['synonym']) . ', ' . json_encode($section['start']) . ', '
		. json_encode($section['end']) . ', '
		. json_encode(array('m' => !empty($section['days'][0]), 't' => !empty($section['days'][1]), 'w' => !empty($section['days'][2]),
				    'h' => !empty($section['days'][3]), 'f' => !empty($section['days'][4]),
				    's' => !empty($section['days'][5])))
		. ', ' . json_encode($section['professor']) . ', '
		. json_encode($section['location']) . ', '
		. json_encode($section['type']) . ', '
		. json_encode($section['slot']) . ', '
		. json_encode(isset($section['credit_hours']) ? $section['credit_hours'] : -1) . ', '
		. json_encode(empty($section['date_start']) ? NULL : $section['date_start']) . ', '
		. json_encode(empty($section['date_end']) ? NULL : $section['date_end']) . ');' . PHP_EOL;
	  $my_hc .= PHP_EOL;
	}
  }
else
  {
    $default_courses = school_default_courses($school);
    foreach ($default_courses as $default_class)
      $my_hc .= input_course_js($default_class, '    ');
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

if ($school['id'] != 'default'
    && empty($_REQUEST['selectschool'])
    && empty($_REQUEST['selectsemester']))
  {
    /*
     * If we have chosen a school, set the canonical URL so that it
     * contains the school and, optionall, the specified
     * schedule. This way, when Google caches the input.cgi page, the
     * <title/> it sees will reflect the selected school.
     */
    $query = array('school' => $school['id']);
    if ($sch)
      {
	$query['s'] = $sch->id_get();
	/*
	 * When editing a schedule, also canonize on the
	 * semester. Changes to the selected schedule which are made
	 * when editing an existing schedule are not saved into the
	 * SESSION. Thus, for the user to be able to edit a schedule
	 * and load courses from an alternate semester, we must ensure
	 * that semester remains in GET.
	 */
	if (!empty($semester))
	  $query['semester'] = $semester['id'];
      }
    $inputPage->canonize('input.cgi', $query);
  }

$inputPage->head();

/*
 * Force a student to choose a school or declare he's a generic
 * student before displaying the input form. To do this, we need
 * another variable in $_SESSION: $_SESSION['school_chosen'].
 */
if (!empty($_REQUEST['school']) && !empty($_SESSION['school']) && !strcmp($_REQUEST['school'], $_SESSION['school']))
  $_SESSION['school_chosen'] = TRUE;
if (!empty($_REQUEST['selectschool'])
    || empty($school) || $school['id'] == 'default' && empty($_SESSION['school_chosen']))
  {
    $next_page = 'input.cgi?';
    if (isset($_GET['s']))
      $next_page .= 's=' . (int)$_GET['s'] . '&';
    if (isset($_GET['semester']))
      $next_page .= 'semester=' . htmlentities($$_GET['semester']) . '&';
?>
<h2>School Selection</h2>
<p>
  Choose the school you attend from the list below. <strong>If you cannot
  find your school</strong>, you may proceed using
  the <a href="<?php echo htmlentities($next_page); ?>school=default">generic
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
  href="feedback.cgi?feedback=My+school+is+missing+the+&lt;semester+name&gt;+semester.">let us know</a>.
</p>
<?php
  $next_page = 'input.cgi?';
  if (isset($_GET['s']))
    $next_page .= 's=' . (int)$_GET['s'] . '&';
  if (!empty($_GET['school']))
    $next_page .= 'school=' . $_GET['school'] . '&';

  $inputPage->showSemesters($next_page);
  $inputPage->foot();
  exit;
  }

$inputPage->showSavedScheds($_SESSION);
?>

<?php if (!empty($input_warning_banner)): ?>
<div class="warning">
  <?php echo $input_warning_banner; ?>
</div>
<?php endif; ?>

<p>
  Welcome to SlatePermutate<?php $inputPage->addressStudent(', ', '', FALSE); ?>!
  <?php if (school_has_auto($inputPage->get_school())): ?>
  To get started, enter in a course identifier (e.g., <em>
  <?php echo empty($semester) || empty($semester['popular_course_id']) ? school_example_course_id($inputPage->get_school()) : $semester['popular_course_id']; ?></em>)
  and click the autosuggestion to automatically load available sections
  for each class. (Please note that autosuggestion data may occasionally fall out of sync with your registrar’s records).
  <?php else: ?>
  To get started, enter a course number and add some sections to it.
  Then specify each section's letter/number and what times it meets,
  add more courses, and click &ldquo;Find a Schedule&rdquo;.
  <!--'-->
  <?php endif; ?>
</p>

<form method="post" action="process.cgi" id="scheduleForm">
<p class="nospace" style="border-left: 5px solid #999; padding-left: 5px!important; padding-top: 5px!important;"><label>Schedule Name</label><br />
<input
    id="scheduleName"
    style="margin-bottom: 1em;"
    class="defText required input-submit-disable"
    type="text"
    size="25"
    title="My <?php echo $semester['name']; ?> Schedule"
    name="postData[name]"
    <?php
      if ($sch)
        echo 'value="' . htmlentities($sch->getName(), ENT_QUOTES) . '"';
      elseif ($errors_fix)
        echo 'value="' . htmlentities($_POST['postData']['name'], ENT_QUOTES) . '"';
    ?> />
  <?php if (!empty($parent_schedule_id)): ?>
  <input type="hidden" name="postData[parent_schedule_id]" value="<?php echo htmlentities($parent_schedule_id, ENT_QUOTES); ?>" />
  <input type="hidden" name="postData[school]" value="<?php echo htmlentities($school['id']); ?>" />
  <input type="hidden" name="postData[semester]" value="<?php echo htmlentities($semester['id']); ?>" />
  <?php endif; ?>
</p>

<div id="container">
      <table id="jsrows">
	<!-- Allow CSS to apply to entire rows at a time. -->
	<colgroup>
	  <col />
	  <col />
	  <col />
	  <col />
	  <col />
	  <col class="sunday<?php if (school_has_auto($inputPage->get_school())) echo ' collapsed';?>" />
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
	  <td class="center">Su</td>
          <td class="center">M</td>
          <td class="center">Tu</td>
          <td class="center">W</td>
          <td class="center">Th</td>
          <td class="center">F</td>
	  <td class="center">Sa</td>
          <td class="center"></td>
          <td class="center"></td>
        </tr>
      </table>
</div>

<div class="credit-hours-total">
  <p>Credit Hours: <span class="credit-hours-total-value">0</span></p>
</div>

<div class="paddingtop">
  <input class="button olive" type="submit" value="Find a schedule" />
</div>

</form>

<?php 

/* Show/hide Advanced Options: <p><span id="showadvanced" style="margin-left: 1em;"><a href="#">Advanced</a></span></p> */ 
?>
<div id="showInstructions" style="width: 100%; text-align: center;"><a href="#">Detailed Instructions…</a></div>

<?php
$inputPage->showSchoolInstructions();
$inputPage->foot();

function input_course_js(Course $course, $whitespace = '  ')
{
  $title = $course->title_get();
  if (empty($title))
    $title = '';
  $js = $whitespace . 'class_last = add_class_n(' . json_encode($course->getName()) . ', '
    . json_encode($title) . ');' . PHP_EOL;

  foreach ($course as $course_slot)
    foreach ($course_slot as $section)
      {
	$meetings = $section->getMeetings();
      foreach ($meetings as $meeting)
	{
	  $js .= $whitespace . 'add_section_n(class_last, ' . json_encode($section->getLetter()) . ', '
	    . json_encode($section->getSynonym()) . ', '
	    . json_encode($meeting->getStartTime()) . ', '
	    . json_encode($meeting->getEndTime()) . ', '
	    . json_encode(array('u' => $meeting->getDay(6), 'm' => $meeting->getDay(0), 't' => $meeting->getDay(1),
				'w' => $meeting->getDay(2), 'h' => $meeting->getDay(3), 'f' => $meeting->getDay(4),
				's' => $meeting->getDay(5))) . ', '
	    . json_encode($meeting->instructor_get()) . ', '
	    . json_encode($meeting->getLocation()) . ', '
	    . json_encode($meeting->type_get()) . ', '
	    . json_encode($course_slot->id_get()) . ', '
	    . json_encode($section->credit_hours_get()) .', '
	    . json_encode($meeting->date_start_get()) . ', '
	    . json_encode($meeting->date_end_get()) . ');' . PHP_EOL;
	}
    }

  return $js;
}
