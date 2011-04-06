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

include_once 'inc/class.page.php';

/* Make sure we start our own named session and to not let securimage create its own :-p */
page::session_start();

if ($use_captcha)
  {
    require_once 'securimage/securimage.php';
    $securimage = new Securimage();
  }

$feedbackpage = page::page_create('Feedback');
$feedbackpage->head();
$subject = '[SlatePermutate] - Feedback';
?>

<h3>Thanks!</h3>

<?php

$ip = $_POST['ip'];
$httpagent = $_POST['httpagent'];
$fromdom = $_POST['fromdom'];
$nameis = $_POST['nameis'];
$visitormail = $_POST['visitormail'];
$school = $_POST['school'];
$school_id = isset($_SESSION['school']) ? $_SESSION['school'] : '';
$feedback = $_POST['feedback'];
$rating = $_POST['rating'];
$referrer = $_POST['referrer'];

$saved_schedules = array();
if (!empty($_SESSION['saved']))
  foreach ($_SESSION['saved'] as $key => $val)
    $saved_schedules[] = $key;
$saved_schedules = implode(', ', $saved_schedules);

$reject = FALSE;

if (eregi('http:', $feedback)) { 
  echo '<p>Please do not include URLs in your submission! Please click "back" and try again.</p>';
  $reject = TRUE;
}
if (empty($visitormail) || !preg_match('/^[^@]+@[^@]+\.[^@]+$/', $visitormail)) {
  echo '<p>Please click "back" and enter valid e-mail address.</p>';
  $reject = TRUE;
}
if(empty($nameis) || empty($feedback) || empty($visitormail)) {
  echo '<p>Please click "back" and fill in all fields.</p>';
  $reject = TRUE;
}

/** Check the captcha */
if ($use_captcha)
  {
    if (!$securimage->check($_REQUEST['captcha_code']))
      {
	echo '<p>Your captcha response was incorrect, please try again.</p>';
	$reject = TRUE;
      }
  }

if (!$reject)
  {
    $feedback = stripcslashes($feedback);

    $message = date('l, F j, Y, g:i a') ."
From: $nameis ($visitormail)
School: $school ($school_id)\n
Rating: $rating 
Feedback: $feedback 
\n
IP = $ip 
Browser = $httpagent 
Deployment = $fromdom
Referrer = $referrer
saved_schedules = $saved_schedules
";

    $from = "From: $visitormail\r\n";

    /* $feedback_emails has its default set in inc/class.page.inc, can be set in config.inc */
    foreach($feedback_emails as $toaddr)
      {
	mail($toaddr, $subject, $message, $from);
      }

    if($feedback_disk_log) {
      $file = fopen($feedback_disk_log_file,'a') or die("Can't open file.");
      fwrite($file, $message . "----------------------------------------\n");
      fclose($file);
    }
    echo '<p>Thanks for helping make SlatePermutate better. Your feedback is greatly appreciated.</p>';
    echo '<p>We will attempt to respond via email if your feedback lends itself to a response.</p>';
  }
    $feedbackpage->foot();
