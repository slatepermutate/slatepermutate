<?php 
	include_once 'inc/class.page.php'; 
	$mypage = new page('TODO');
?>

<h3>Items to Consider:</h3>
<ul>
	<li>Add place to put prof name</li>
	<li>Autoincrement section num/letter/custom labels</li>
	<li>Make output and print output formatting look nicer</li>
	<li>Make printing work for saved jobs where jobkey != 0</li>

	<li>After selecting a start time, set the end time to one hour after the start time</li>
        <li><strong>Append</strong> sections</li>
        <li>Move the add class button to somewhere nicer, maybe a gray row at the bottom. Make the submit button more obvious.</li>
	<li>Form validation to ensure endtime is after starttime, at least one day is checked.</li>
	<li>Auto-populate form based on saved schedule?</li>

        <li>Grab data from school sites such as <a href="http://www.cedarville.edu/courses/schedule/2010fa_be_bebl.htm" rel="external">this?</a></li>
</ul>


<?php
$mypage->foot();
