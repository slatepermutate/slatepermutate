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

$inc_dir = dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;
require_once $inc_dir . 'class.page.php';

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
	       'example_course_id' => 'ENGL-101',
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

/**
 * \brief
 *  A generic fallback for school_registration_html().
 *
 * \see school_registration_html()
 */
function default_registration_html(Page $page, array $school, array $courses)
{
  if (!empty($school['registration_url']))
    {
      $link_url = $school['registration_url'];
      $link_text = $school['name'] . '\'s registration system';
    }
  else
    {
      $link_url = $school['url'];
      $link_text = $school['name'] . '\'s website';
    }

  $html = ''
    . '  <p>' . PHP_EOL
    . '    Enter these codes into ' . htmlentities($school['name']) . '\'s online course registration' . PHP_EOL
    . '    system (<a href="' . htmlentities($link_url) . '" target="_blank">' . htmlentities($link_text) . '</a>)' . PHP_EOL
    . '    to register for classes:' . PHP_EOL
    . '  </p>' . PHP_EOL
    . '  <ul class="synonym-list">' . PHP_EOL;
  foreach ($courses as $course)
    foreach ($course as $course_slot)
      foreach ($course_slot as $section)
        $html .= '    <li>' . htmlentities($section->getSynonym()) . '</li>' . PHP_EOL;
  $html .= '  </ul>' . PHP_EOL;
  return $html;
}
