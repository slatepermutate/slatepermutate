#!/usr/bin/env php-cgi
<?php /* -*- mode: php; -*- */
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

if (isset($_GET['success']))
  {
    echo '<h3>Thanks</h3>' . PHP_EOL
      . '<p>Thanks for helping make SlatePermutate better. Your feedback is greatly appreciated.</p>' . PHP_EOL
      . '<p>We will attempt to respond via email if your feedback lends itself to a response.</p>' . PHP_EOL;
    $feedbackpage->foot();
    exit();
  }

$subject = '[SlatePermutate] - Feedback';

$ip = $_SERVER['REMOTE_ADDR'];
$httpagent = $_SERVER['HTTP_USER_AGENT'];
$user_supplied_params = array('fromdom', 'nameis', 'visitormail', 'school', 'feedback', 'rating', 'referrer');
foreach ($user_supplied_params as $var)
  {
    if (isset($_POST[$var]))
      ${$var} = $_POST[$var];
    else
      /* Obviously, the user has not actually  */
      page::redirect('feedback.cgi');
  }
$school_id = isset($_SESSION['school']) ? $_SESSION['school'] : '';

$saved_schedules = array();
if (!empty($_SESSION['saved']))
  foreach ($_SESSION['saved'] as $key => $val)
    $saved_schedules[] = $key;
$saved_schedules = implode(', ', $saved_schedules);

$reject = FALSE;
$messages = '';

if (preg_match('/https?:/i', $feedback)) { 
  $messages .= '<p>Please do not include URLs in your submission!</p>' . PHP_EOL;
  $reject = TRUE;
}
if (empty($visitormail) || !preg_match('/^[^@]+@[^@]+\.[^@]+$/', $visitormail)
    || !($visitormail = filter_var($visitormail, FILTER_VALIDATE_EMAIL)))
  {
  $messages .= '<p>Please enter a valid e-mail address.</p>' . PHP_EOL;
  $reject = TRUE;
}
if(empty($nameis) || empty($feedback) || empty($visitormail)) {
  $messages .= '<p>You must fill in in all of the fields.</p>' . PHP_EOL;
  $reject = TRUE;
}

/** Check the captcha */
if ($use_captcha)
  {
    if (empty($_REQUEST['captcha_code'])
	|| !$securimage->check($_REQUEST['captcha_code']))
      {
	$messages .= '<p>Your captcha response was incorrect or expired.</p>';
	$reject = TRUE;
      }
  }

$success = FALSE;
// Ignore the feedback if it claims to be from @slatepermutate.org like some recent spam.
if (preg_match('/@slatepermutate\\.org/', $visitormail)) {
  // Just ignore but pretend we didn’t.
  $success = TRUE;
}
if (!$success && !$reject)
  {
    $feedback = stripcslashes($feedback);

    $message = gmdate('l, F j, Y, g:i a') ."
Reply-To: $nameis <$visitormail>
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
	$success = mail($toaddr, $subject, $message, $from);
	if (!$success)
	  {
	    $messages .= '<p>This Slate Permutate installation is misconfigured and unable to send email. Please contact the administrator of this website using a more direct means if possible.</p>' . PHP_EOL;
	  }
      }

    if($feedback_disk_log) {
      $file = fopen($feedback_disk_log_file,'a') or die("Can't open file.");
      fwrite($file, $message . "----------------------------------------\n");
      fclose($file);
    }
  }
if ($success)
  page::redirect('feedback-submit.cgi?success');
else
  echo '<h3>Error</h3>' . PHP_EOL
    . $messages;

$repost = array();
foreach ($user_supplied_params as $user_supplied_param)
  $repost[$user_supplied_param] = $_POST[$user_supplied_param];
echo $feedbackpage->query_formbutton('feedback.cgi', $repost, $feedbackpage->entities('try again'), '<p>Consider the error messages, then ', '.</p>');

$feedbackpage->foot();
