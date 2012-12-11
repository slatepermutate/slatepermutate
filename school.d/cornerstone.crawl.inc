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

require_once dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR . 'school.crawl.webadvisor.inc';

function cornerstone_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  return school_crawl_webadvisor_semester_list($school, $semesters, $school_crawl_log, array('curlsetup_hook' => '_' . $school['id'] . '_crawl_semester_curlsetup_hook'));
}

function cornerstone_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  return school_crawl_webadvisor_semester($school, $semester, $school_crawl_log, array('curlsetup_hook' => '_' . $school['id'] . '_crawl_semester_curlsetup_hook'));
}

/**
 * \brief
 *   Tell libCURL to ignore the missing SSL cert in the misconfigured
 *   server’s SSL certchain.
 *
 * https://solomon.cornerstone.edu/ is missing the middle cert from
 * its SSL certchain. The only way to expose this is to remove the
 * cached cert from Mozilla’s certcache. Visiting
 * https://eaglesnest.cornerstone.edu/ before visiting solomon is
 * enough to get the missing cert cached into Mozilla’s certcache. We
 * could try to emulate this behavior, but PHP’s openssl library
 * doesn’t support SSL connections (it just does encrypt/decrypt and
 * hashes) and it seems libcurl provides no way to access/save the
 * certs it encounters, though it does provide a way to specify CA
 * certs. I just don’t feel like adding the intermediate CA cert
 * slate_permutate’s repository… so, now for some untrusted computing
 * instead of getting the verified intermediate cert from eaglesnest!
 */
function _cornerstone_crawl_semester_curlsetup_hook($curl)
{
  curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, FALSE);
}
