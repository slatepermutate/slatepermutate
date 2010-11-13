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
	$mypage = new page('Welcome');
?>

<h3>Find the schedule that works for you!</h3>
<p>View <a href="schedulecreator.php">demo output</a> or <a href="input.php">get started on your own</a>. This program was created by <a href="http://www.calvin.edu" target="_blank">Calvin College</a> and <a href="http://cedarville.edu/" target="_blank">Cedarville University</a> students. SlatePermutate works with any college or university.</p>
<p class="righttext"><a href="input.php"><img class="noborder" src="images/get-started.png" alt="Get Started" /></a></p>

<?php
$mypage->foot();
