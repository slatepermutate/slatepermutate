<?php /* -*- mode: php; -*- */
/*
 * Copyright 2011 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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

function umd_info()
{
  return array('name' => 'University of Maryland',
	       'url' => 'http://umd.edu/',
	       'domains' => array(
				  'umd.edu',
				  ),
	       'example_course_id' => 'CMSC106',
	       'student_address' => 'student',
	       );
}

function umd_instructions_html()
{
  /* ``inherit'' stuff from the default school. */
  $default_school = school_load('default');
  $school_info = umd_info();

  return <<<EOF
<h2>UMD Notes</h2>
<p>
  We do not yet have support autocomplete for the <a
  href="$school_info[url]">$school_info[name]</a>. You may find <a
  href="http://ourumd.com/">ourumd's</a><!--'--> <a
  href="http://ourumd.com/sched/">Schedule Maker</a> more useful than
  slate_permutate &mdash; even if we did have autocomplete support for
  $school_info[name].
</p>
EOF
		. default_instructions_html();
}
