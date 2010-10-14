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

function default_info()
{
  return array('name' => 'Generic College',
	       'url' => 'http://example.edu/',
	       /*
		* rDNS postfixes which indicate a student is
		* connecting from that college campus.
		*/
	       'domains' => array(
				  ),
	       'student_address' => 'student',
	       );
}

function default_instructions_html()
{
  return <<<EOF
<h2>Generic Instructions</h2>
<p>
  <tt>slate_permutate</tt> can be a useful tool for scheduling your next semester at the college you attend.
</p>
<ol>
  <li>Get in touch with your advisor and find out what courses you need to take next semester.</li>
  <li>Look up each course which your advisor specified in your college's course catalog. Note the different sections for each course..</li>
  <li>Enter each course into a <tt>slate_permutate</tt> schedule. For each course, add the sections listed in your school's course catalogue (at your discretion).</li>
  <li>Submit your list of courses and their sections. Let <tt>slate_permutate</tt> perform its magic.</li>
  <li>View all of the different permutations of your schedule and find a few that work for you.</li>
  <li>Print out your preferred schedule by selecting a schedule an choosing your User-Agent's print option.</li>
  <li>Wait until it's your turn to register and grab your preferred sections before they fill up!</li>
</ol>
EOF;
}
