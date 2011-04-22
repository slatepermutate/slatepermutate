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

function umich_info()
{
  return array('name' => 'University of Michigan',
               'url' => 'http://umich.edu/',
               'domains' => array(
                                  'umich.edu',
                                  ),
	       'example_course_id' => 'MATH-161',
               'student_address' => 'Wolverine',
               );
}

function umich_instructions_html()
{
  return <<<EOF
<h2>U of M-specific Instructions</h2>
<p>
  <em>Note that there is another scheduler targeted particularly at U
  of M students, <a href="http://mschedule.com/" title="Java
  applets... Ew!">MSchedule</a>.
</p>
<ol>
  <li>Get in touch with your adviser during advising/reading recess.</li>
  <li>Look up each class your adviser specified on this course listing page</li>
  <li>Enter each class into a SlatePermutate schedule and add each section that is listed that you are willing to take.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule which would work with the sections you specified.</li>
  <li>Print out your preferred schedule by choosing "print" and selecting a schedule.</li>
  <li>Wait until it's your turn to register and grab your preferred sections before they fill up!</li>
</ol> <!-- ' -->
EOF;
}
