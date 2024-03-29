<?php /* -*- mode: php; -*- */
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
  return array(
    'name' => 'Calvin University',
    'url' => 'https://calvin.edu/',
    /*
     * rDNS postfixes which indicate a student is
     * connecting from that college campus
     */
    'domains' => array(
      'calvin.edu',
    ),
    'example_course_id' => 'CS-232',
    'registration_url' => 'https://workday.calvin.edu/',
    'student_address' => 'Knight',
    'webadvisor_url' => 'https://resources.calvin.edu/selfservice/WebAdvisor',
  );
}

function calvin_instructions_html()
{
  return <<<EOF
<h2>Instructions</h2>
<h3>Using Auto-Complete</h3>
<ol>
  <li>Click/tap on a the Class ID box in an empty row.</li>
	<li>Type the subject and course number (e.g. MATH 101, CS 104) in the Class ID box.</li>
  <li>Press enter to choose the first option or select one of the options to load data for that course.</li>
  <li>To reload the data, first remove the cource using the “Remove” button and then re-enter the subject and course number. Auto-Complete will not overwrite a populated entry.</li>
 </ol>
<h3>Registering</h3>
<ol>
  <li>Schedule an appointment with your advisor during Advising Break.</li>
  <li>Your advisor will help you select courses to advance your education and mark you as “advised” in Calvin’s registration system.</li>
  <li>Use Slate Permutate (this website) to check out different schedules and sections for the courses you plan to take.</li>
  <li>Look up your <a href="https://calvin.edu/offices-services/center-for-student-success/registration/registration-times/">Registration Time</a>.</li>
  <li>Prior to your registration time, you may pre-search for your desired sections in Calvin’s registration system. The system may let you create and modify a saved list of sections.</li>
  <li>Once your registration time comes, register.</li>
</ol>
EOF;
}
