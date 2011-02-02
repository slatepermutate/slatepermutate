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

require_once 'inc/class.page.php'; 

$welcomepage = page::page_create('Welcome');
$welcomepage->head();
?>

<h3>Find the schedule that works for you!</h3>
<p>Plan your next semester with SlatePermutate! SlatePermutate generates every possible schedule with the courses you enter to let you pick the schedule that fits your life.</p>
<p><a href="input.php">Get started</a></p> 

<p class="righttext"><a href="input.php"><img class="noborder" src="images/get-started.png" alt="Get Started" /></a></p>

<?php
$welcomepage->foot();
