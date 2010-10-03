<?php 
	include_once 'inc/class.page.php'; 
	$mypage = new page('Welcome');
?>

<h3>Find the schedule that works for you!</h3>
<p>View <a href="schedulecreator.php">demo output</a> or <a href="input.php">get started on your own</a>. This program was created by <a href="http://www.calvin.edu">Calvin College</a> and <a href="http://cedarville.edu/">Cedarville University</a> students. SlatePermutate works with any college or university.</p>
<p class="righttext"><a href="input.php"><img class="noborder" src="images/get-started.png" alt="Get Started" /></a></p>

<?php
$mypage->foot();
