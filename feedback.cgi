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

include_once 'inc/class.page.php';
require_once 'inc/class.schedule.php';

$feedbackpage = page::page_create('Feedback');
$feedbackpage->head();

$fromdom = $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];

$referrer = '';
if (!empty($_SERVER['HTTP_REFERER']))
  $referrer = $_SERVER['HTTP_REFERER'];
if (!empty($_POST['referrer']))
  $referrer = $_POST['referrer'];

$saved_schedules = array();
if (!empty($_SESSION['saved']))
  foreach ($_SESSION['saved'] as $key => $val)
    $saved_schedules[] = '<a href="' . htmlentities(Schedule::url($key)) . '">' . htmlentities($key) . '</a>';
$saved_schedules = implode(', ', $saved_schedules);

/* some prefill support */
$school = $feedbackpage->get_school();
$feedback_text = '';
if (isset($_GET['feedback']))
  $feedback_text = $_GET['feedback'];

$n = "\n";

?>
<p>
  Please send feedback using one of the options below.
  We will try to get back with you as soon as we can, but will not always be able to respond immediately.
  So please try to provide as much information (school, semester, what courses you entered) as you can and a full question so that we can act on it when we can.
</p>
<h2>Options</h2>
<ul>
  <li><a class="slatepermutate-link" href="#email">Contact Email Form (below)</a></li>
  <!--
  On Android, screen_name=SlatePermutate is ignored when opening the link in the app.
  The iOS and web versions of Twitter support screen_name.
  So thus the ugly Android-specific, suboptimal link :-/.
  <?php
  $tweet_content = !empty($referrer) || empty($school) || $school['id'] === 'default' ? '' : ('school ' . $school['id'] . ' ');
?>
  -->
  <li><a class="twitter-link" href="https://twitter.com/intent/tweet?screen_name=SlatePermutate<?php if (!empty($referrer)) echo page::entities('&url=' . rawurlencode($referrer)); ?>&amp;text=<?php echo page::entities(rawurlencode($tweet_content));?>">Twitter</a> (Android users <a href="https://twitter.com/intent/tweet?text=<?php echo page::entities(rawurlencode('@SlatePermutate ' . $tweet_content)); ?>">use alternate</a>)</li>
  <li><a class="line-link" href="https://line.me/ti/p/%40ncz3026b">LINE</a></li>
  <li><a class="messenger-link" href="https://m.me/slatepermutate">Facebook Messenger</a></li>
  <li><span class="stackexchange-webapps-link">Consider helping others by <a href="https://webapps.stackexchange.com/questions/ask?tags=slate-permutate">asking</a> general “How do I…?” questions at <a href="https://webapps.stackexchange.com/questions/tagged/slate-permutate">StackExchange Web Applications</a></span></li>
</ul>

<h2 id="email">Contact Email Form</h2>

<form action="feedback-submit.cgi" method="post">
<div id="feedback-form-content">
<input type="hidden" id="fromdom" name="fromdom" value="<?php echo htmlentities($fromdom, ENT_QUOTES); ?>" />

<table>
<tr><td><label for="nameis">Name: </label></td><td><input type="text" id="nameis" name="nameis" size="20" required="required" value="<?php echo empty($_REQUEST['nameis']) ? '' : page::entities($_REQUEST['nameis']);?>" /></td></tr>
<tr><td><label for="visitormail">Email:</label></td><td><input type="email" id="visitormail" name="visitormail" size="20" required="required" value="<?php echo empty($_REQUEST['visitormail']) ? '' : page::entities($_REQUEST['visitormail']);?>" /></td></tr>
<tr><td><label for="school">School: </label></td><td><input type="text" id="school" name="school" value="<?php echo htmlentities($school['id']); ?>" size="20" /> <span class="graytext">(if relevant to your feedback)</span></td></tr>
  <tr><td><label for="referrer">Relevant Page:</label></td><td><input type="text" id="referrer" name="referrer" value="<?php echo htmlentities($referrer); ?>" size="20" /> <span class="graytext">(if relevant to your feedback)</span></td></tr>
</table>
<br/>
<div id="ratings">
  <div id="ratings-label">Overall Rating:</div>
    <div class="radio-spans">
  <span><input checked="checked" id="rating-great" name="rating" type="radio" value="Great" /><label for="rating-great">Great</label></span>
  <span><input id="rating-usable" name="rating" type="radio" value="Usable" /><label for="rating-usable">Usable</label></span>
  <span><input id="rating-buggy" name="rating" type="radio" value="Buggy/Hard to Use" /><label for="rating-buggy">Buggy/Hard to Use</label></span>
  <span><input id="rating-unknown" name="rating" type="radio" value="Don't know" /><label for="rating-unknown">Don't Know <!-- ' --></label></span>
  </div>
</div>
<h3>General Comments</h3>
<p>
  <textarea name="feedback" required="required" rows="6" cols="40"><?php echo htmlentities($feedback_text); ?></textarea>
</p>

<?php
    if ($use_captcha)
    {
      echo '' . $n
      . '  <h3>Captcha</h3>' . $n
      . '<p>' . $n
      . '  <img id="captcha_img" src="captcha_img.cgi" alt="captcha image" /><br />' . $n
      . '  <label for="captcha_code">Enter the obfuscated text from the above image:</label><br />' . $n
      . '  <input id="captcha_code" name="captcha_code" type="text" />' . $n
      . '</p>' . $n;
    }
?>

<button class="gray">Send Feedback</button>

<?php if (!empty($saved_schedules)): ?>
<p class="graytext" style="margin-top: 20pt;">
  The following information will also be submitted when you send feedback:
</p>
<table class="graytext">
  <tr>
    <th>Type</th>
    <th>Value</th>
  </tr>
  <tr>
  <td>Saved Schedules:</td>
    <td><?php echo $saved_schedules; ?></td>
  </tr>
</table>
<?php endif; ?>
</div>
</form>

<?php
$feedbackpage->foot();
