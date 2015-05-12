<?php /* -*- mode: php; -*- */
/*
 * Copyright 2012 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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

require_once dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR . 'school.crawl.oracle.inc';

/**
 * \brief
 *   Obtain list of crawlable semesters offered by CCBCMD.
 *
 * \parram $school
 *   The CCBCMD school handle.
 * \param $semesters
 *   Array to populate with available semesters.
 * \return
 *   0 on success.
 */
function ccbcmd_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  return school_crawl_oracle_semester_list($school, $semesters, $school_crawl_log);
}

/**
 * \brief
 *   Crawl a CCBCMD semester.
 *
 * \param $school
 *   The CCBCMD school handle.
 * \param $semester
 *   The semester to fill with courses.
 */
function ccbcmd_crawl_semester($school, $semester, &$school_crawl_log)
{
  return school_crawl_oracle_crawl_semester($school, $semester, $school_crawl_log);
}
