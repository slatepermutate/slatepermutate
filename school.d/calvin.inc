<?php
/*
 * Copyright 2010 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
 *
 * This file is a part of slate_permutate.
 *
 * slate_permutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * slate_permutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with slate_permutate.  If not, see <http://www.gnu.org/licenses/>.
 */

function calvin_info()
{
  return array('name' => 'Calvin College',
	       'url' => 'http://calvin.edu/',
	       /*
		* rDNS postfixes which indicate a student is
		* connecting from that college campus
		*/
	       'domains' => array(
				  'calvin.edu',
				  ),
	       'student_address' => 'Knight',
	       );
}

function calvin_instructions_html()
{
  return <<<EOF
<h2>Instructions</h2>
<!--<p>
  SlatePermutate can be a useful tool for scheduling your next semester at <a href="http://calvin.edu/">Calvin College</a>.
</p>-->
<h3>
	Using the Auto-Complete function
</h3>
<ol>
  <li>Determine the classes you wish to take next semester.</li>
	<li>Enter in the class numbers (e.g. MATH 101, CS 104) in the Class ID box.</li>
	<li>Select a valid class number from the drop down menu.</li>
	<li>Click anywhere outside of the box to automatically fill in the available section data for that class.</li>
	<li>Repeat this process for each class.</li>
	<li>Click "Find a Schedule" to view all possible schedules for next semester.</li>
</ol>
<!--<ol>
  <li>Get in touch with your advisor during advising/reading recess.</li>
  <li>Look up each class your advisor specified in Calvin's registration page of <a href="http://kv.calvin.edu/">KnightVision</a> (soon to be <a href="http://moodle.calvin.edu">moo</a>).</li>
  <li>Enter each class into a <tt>slate_permutate</tt> schedule and add each section that is listed that you are willing to take.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule which would work with the sections you specified.</li>
  <li>Print out your preferred schedule by choosing "print" and selecting a schedule.</li>
  <li>Wait until it's your turn to register and grab your preferred sections before they fill up!</li>
</ol>-->
EOF;
}
