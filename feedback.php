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

$feedbackpage = page::page_create('Feedback');
$feedbackpage->head();
$ipi = $_SERVER['REMOTE_ADDR'];
$fromdom = $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
$httpagenti = $_SERVER['HTTP_USER_AGENT'];

/* some prefill support */
$school = $feedbackpage->get_school();
$feedback_text = '';
if (isset($_GET['feedback']))
  $feedback_text = $_GET['feedback'];

$n = "\n";

?>

<form action="feedback-submit.php" method="post">
<input type="hidden" name="ip" value="<?php echo $ipi ?>" />
<input type="hidden" name="fromdom" value="<?php echo $fromdom ?>" />
<input type="hidden" name="httpagent" value="<?php echo $httpagenti ?>" />

<h2>Feedback Form</h2>
<label for="nameis">Name: </label><input type="text" name="nameis" size="20" /><br />
<label for="visitormail">Email:&nbsp; </label><input type="text" name="visitormail" size="20" /> <span class="graytext">(if you want us to get back to you)</span><br />
<label for="school">School: </label><input type="text" name="school" value="<?php echo htmlentities($school['id']); ?>" size="20" /> <span class="graytext">(if relevant to your feedback)</span><br />

<br/> Overall Rating:<br/> <input checked="checked" name="rating" type="radio" value="Good" />Good <input name="rating" type="radio" value="Buggy" />Buggy  <input name="rating" type="radio" value="Needs more features" />Needs more features <input name="rating" type="radio" value="Don't know" />Don't Know <!-- ' -->

<br /><br />
<h3>General Comments</h3>
<p>
  <textarea name="feedback" rows="6" cols="40"><?php echo htmlentities($feedback_text); ?></textarea>
</p>

<?php
    if ($use_captcha)
    {
      echo '' . $n
      . '  <h3>Captcha</h3>' . $n
      . '<p>' . $n
      . '  <img id="captcha_img" src="captcha_img.php" alt="captcha image" /><br />' . $n
      . '  <label for="captcha_code">Enter the obfuscated text from the above image:</label><br />' . $n
      . '  <input name="captcha_code" type="textbox" />' . $n
      . '</p>' . $n;
    }
?>

<input class="gray" type="submit" value="Submit Feedback" />
</form>

<?php
$feedbackpage->foot();
