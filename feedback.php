<?php 
	include_once 'inc/class.page.php'; 
	$mypage = new page('Feedback');

	$ipi = $_SERVER['REMOTE_ADDR'];
	$fromdom = $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
	$httpagenti = $_SERVER['HTTP_USER_AGENT'];
?>
<form action="feedback-submit.php" method="post">
<input type="hidden" name="ip" value="<?php echo $ipi ?>" />
<input type="hidden" name="fromdom" value="<?php echo $fromdom ?>" />
<input type="hidden" name="httpagent" value="<?php echo $httpagenti ?>" />
<h2>Feedback Form</h2>
<label for="nameis">Name: </label><input type="text" name="nameis" size="20" /><br />
<label for="visitormail">Email:&nbsp; </label><input type="text" name="visitormail" size="20" /> <span class="graytext">(if you want us to get back to you)</span><br />
<label for="school">School: </label><input type="text" name="school" size="20" /> <span class="graytext">(if relevant to your feedback)</span><br />


<br/> Overall Rating:<br/> <input checked="checked" name="rating" type="radio" value="Good" />Good <input name="rating" type="radio" value="Buggy" />Buggy  <input name="rating" type="radio" value="Needs more features" />Needs more features <input name="rating" type="radio" value="Don't know" />Don't Know

<br /><br />
<h3>General Comments</h3>
<p>
<textarea name="feedback" rows="6" cols="40"></textarea>
</p>
<input class="gray" type="submit" value="Submit Feedback" />
</form>






<?php
$mypage->foot();
