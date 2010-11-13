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
