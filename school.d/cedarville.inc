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

function cedarville_info()
{
  return array('name' => 'Cedarville University',
	       'url' => 'http://cedarville.edu/',
	       'domains' => array(
				  'cedarville.edu',
				  ),
	       'example_course_id' => 'MATH-2510',
	       'registration_url' => 'http://cedarville.edu/cgi-bin/secure/register_student.pl',
	       'student_address' => 'Cedarville Student',
	       );
}

function cedarville_instructions_html()
{
  return <<<EOF
<h2>Instructions</h2>
<p>
  SlatePermutate can be a useful tool for scheduling your next semester at <a href="http://cedarville.edu/" rel="external">Cedarville University</a>.
</p>
<ol>
  <li>Enter the course ID, such as PHYS-1020, in the Class ID blank. You will see a list of auto-suggestions.</li>
  <li><strong>You must click on the auto-suggested item</strong> to automatically add all sections of the class.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule.</li>
  <li><strong>Schedule a meeting with your advisor to review your schedule.</strong></li>
  <li>When it's time to register, check the "Show Synonyms" box on your schedule and enter your course synonyms into the registration interface.</li>
</ol> <!--'-->
EOF;
}

/**
 * \brief
 *   Get a list of default classes (with sections (with meeting
 *   times)) for Cedarville students.
 *
 * \return
 *   An array of Course objects.
 */

function cedarville_default_courses()
{
  $chapel = new Course('Chapel','Chapel');
  $chapel->section_add(new Section('_', array(new SectionMeeting('mtwhf', '1000', '1045', '', 'chapel','n/a')),
				   '', '_'));

  return array($chapel);
}

/**
 * \brief
 *   Implement <school_id>_page_css().
 */
function cedarville_page_css($school)
{
  return <<<CSS
#container .lab .sectionIdentifier {
  background: none!important;
}
#container .lab .sectionIdentifier input {
  display: none;
}
#container .lab .deleteSection input {
  display: none;
}
CSS;
}
