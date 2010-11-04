<?php
        include_once 'inc/class.page.php';
        $feedbackpage = new page('Feedback');

	$subject = '[SlatePermutate] - Feedback';
?>

<h3>Thanks!</h3>


<?php
Page::session_start();

$ip = $_POST['ip'];
$httpagent = $_POST['httpagent'];
$fromdom = $_POST['fromdom'];
$nameis = $_POST['nameis'];
$visitormail = $_POST['visitormail'];
$school = $_POST['school'];
$school_id = isset($_SESSION['school']) ? $_SESSION['school'] : '';
$feedback = $_POST['feedback'];
$rating = $_POST['rating'];

$reject = FALSE;

if (eregi('http:', $feedback)) { 
  echo 'Please do not include URLs in your submission! Please click "back" and try again.';
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
";

    $from = "From: $visitormail\r\n";

    /* $feedback_emails has its default set in inc/class.page.inc, can be set in config.inc */
    foreach($feedback_emails as $toaddr)
      {
	mail($toaddr, $subject, $message, $from);
      }

?>

<p>Thanks for helping make SlatePermutate better. Your feedback is greatly appreciated.</p>
<p>We will attempt to respond via email if your feedback lends itself to a response.</p>


<?php
  }

  $feedbackpage->foot();
