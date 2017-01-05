<?php /* -*- mode: php; -*- */
/*
 * Copyright 2014 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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

function cornerstone_info()
{
  return array(
    'example_course_id' => 'ENGL 101',
    'name' => 'Cornerstone University',
    'student_address' => 'Eagle',
    // Redirects to HTTPS but on a port other than 443, so in the name
    // of security hardcoding that port for now. But maybe we are the
    // reason they changed ports?
    'webadvisor_url' => 'https://webadvisor.cornerstone.edu:8443/WAPROD/WebAdvisor',
  );
}
