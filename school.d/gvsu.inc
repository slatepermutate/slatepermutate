<?php /* -*- mode: php; -*- */
/*
 * Copyright 2015 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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

function gvsu_info()
{
  $domain = 'gvsu.edu';
  return array(
    'name' => 'Grand Valley State University',
    'url' => 'http://' . $domain . '/',
    'domains' => array(
      $domain,
    ),
    'oracle_uri' => 'https://mybanner.gvsu.edu/PROD/bwckschd.p_disp_dyn_sched',
    'student_address' => 'Laker',
  );
}
