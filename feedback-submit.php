<?php
        include_once 'inc/class.page.php';
        $feedbackpage = new page('Feedback');

	$toaddrs = array('ethanzonca@gmail.com, ngelderloos7@gmail.com, ohnobinki@ohnopublishing.net');
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

if (eregi('http:', $feedback)) { 
  echo 'Please do not include URLs in your submission! Please click "back" and try again.';
}
if((!$visitormail == '') && (!strstr($visitormail, '@') || !strstr($visitormail, '.'))) {
  echo '<p>Please click "back" and enter valid e-mail address.</p>';
}
if(empty($nameis) || empty($feedback) || empty($visitormail)) {
  echo '<p>Please click "back" and fill in all fields.</p>';
}


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

foreach($toaddrs as $toaddr){
  mail($toaddr, $subject, $message, $from);
}


?>

<p>Thanks for helping make SlatePermutate better. Your feedback is greatly appreciated.</p>
<p>We will attempt to respond via email if your feedback lends itself to a response.</p>


<?php
  $feedbackpage->foot();
