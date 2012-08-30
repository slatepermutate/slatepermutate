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

function dordt_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
{
  $cookies = array();
  $uri = $school['url'] . 'academics/course_schedules/';
  $semesters_html = school_crawl_geturi($uri, $cookies, $school_crawl_log);
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML($semesters_html);
  $semesters_xpath = new DOMXPath($semesters_dom);
  $seasons_map = array(
    'S' => 'spring',
    'F' => 'fall',
  );
  foreach ($semesters_xpath->query('//a[contains(@href, "course_list.pl?")]') as $a_node)
    {
      $q = array();
      list(, $href_querystring) = explode('?', $href = $a_node->getAttribute('href'));
      foreach (explode('&', $href_querystring) as $href_query_namevalue)
	{
	  list($name, $value) = explode('=', $href_query_namevalue, 2);
	  $q[$name] = $value;
	}

      if (empty($q['year'])
	  || empty($q['sem']))
	{
	  school_crawl_logf($school_crawl_log, 2, "Unable to parse URI's GET arguments into year and sem parts: %s", $href);
	  continue;
	}
      if (empty($seasons_map[$q['sem']]))
	{
	  school_crawl_logf($school_crawl_log, 2, "Unable to parse season `%s' into season for `%s': %s",
			    $q['sem'], $a_node->textContent, $href);
	  continue;
	}

      $semesters[] = new Semester($q['year'], $seasons_map[$q['sem']]);
    }

  return 0;
}

function dordt_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
{
  $seasons_map = array(
    'spring' => 'S',
    'fall' => 'F',
  );
  if (empty($seasons_map[$semester->season_get()]))
    {
      school_crawl_logf($school_crawl_log, 2, "Unable to understand season %s.", $semester->season_get());
      return 1;
    }

  $cookies = array();
  $uri = $school['url'] . 'academics/course_schedules/';
  $semesters_html = school_crawl_geturi($uri, $cookies, $school_crawl_log);
  $semesters_dom = new DOMDocument();
  $semesters_dom->loadHTML($semesters_html);
  $semesters_xpath = new DOMXPath($semesters_dom);
  $semester_href = NULL;
  foreach ($semesters_xpath->query('//a[contains(@href, "course_list.pl?") and contains(@href, "sem=' . $seasons_map[$semester->season_get()] . '") and contains(@href, "year=' . $semester->year_get() . '")]') as $a_node)
      $semester_href = $a_node->getAttribute('href');
  if ($semester_href === NULL)
    {
      school_crawl_logf($school_crawl_log, 4, "Unable to find link associated with schedule's semester");
      return 1;
    }

  $uri = school_crawl_url($uri, $semester_href);
  $semester_html = school_crawl_geturi($uri, $cookies, $school_crawl_log);
  $semester_dom = new DOMDocument();
  $semester_dom->loadHTML($semester_html);
  $semester_xpath = new DOMXPath($semester_dom);

  foreach ($semester_xpath->query('//ul[contains(concat(" ", normalize-space(@class), " "), " columns ")]//a[contains(@href, "#")]')
	   as $a_node)

    {
      /* <li><a href="#ASK">Academic Skills</a></li> */
      list(, $hash) = explode('#', $a_node->getAttribute('href'));
      $semester->department_name_set($hash, $a_node->textContent);
    }

  $labs = array();
  foreach ($semester_xpath->query('//table[contains(concat(" ", normalize-space(@class), " "), " schedule ")]') as $table_schedule)
    {
      $head_tr = NULL;
      foreach ($semester_xpath->query('.//thead//tr', $table_schedule) as $head_tr)
	break;
      if ($head_tr === NULL)
	{
	  school_crawl_logf($school_crawl_log, 4, "Unable to find headings for some table.");
	  continue;
	}

      $column_mapping = array(
	'DEPT' => FALSE,
	'NUM' => FALSE,
	'SEC' => FALSE, /* section */
	'TITLE' => FALSE,
	'CR' => FALSE, /* credits */
	'RM' => FALSE, /* room */
	'INSTRUCTOR' => FALSE,
      );
      $willy = FALSE;
      foreach ($column_mapping as $name => $false)
	if (($column_mapping[$name] = school_crawl_table_resolve_column($head_tr, $name)) === FALSE)
	  {
	    school_crawl_logf($school_crawl_log, 4, "Unable to resolve column %s.", $name);
	    $willy = TRUE;
	    break;
	  }
      if ($willy)
	continue;

	/*
	 * <tr>
	 *   <td>AGRI</td>
	 *   <td>111L</td>
	 *   <td>01</td>
	 *   <td>Agri 111 Lab</td>
	 *   <td>0.00</td>
	 *   <td colspan="2" style="text-align: right;">De Vries, G</td>
	 * </tr>
	 * <tr>
	 *   <td colspan="3"></td>
	 *   <td colspan="2">2:00 PM - 5:00 PM, W</td>
	 *   <td colspan="2" align="left" valign="top">SB 138</td>
	 * </tr>
	*/
      foreach ($semester_xpath->query('.//tbody//tr', $table_schedule) as $tr_row)
	{
	  $rownodes = school_crawl_table_rownodes($tr_row);
	  $new_dept = school_crawl_table_rownode_index($rownodes, $column_mapping['DEPT'])->textContent;

	  if (!empty($new_dept))
	    {
	      $course = school_crawl_table_rownode_index($rownodes, $column_mapping['NUM'])->textContent;
	      $section = school_crawl_table_rownode_index($rownodes, $column_mapping['SEC'])->textContent;
	      $title = school_crawl_table_rownode_index($rownodes, $column_mapping['TITLE'])->textContent;
	      $credits = school_crawl_table_rownode_index($rownodes, $column_mapping['CR'])->textContent;
	      $instructor = school_crawl_table_rownode_index($rownodes, $column_mapping['INSTRUCTOR'])->textContent;
	      $dept = $new_dept;
	    }
	  else
	    {
	      if (empty($dept))
		{
		  school_crawl_logf($school_crawl_log, 6, "Unexpected empty DEPT column");
		  continue;
		}

	      $room = school_crawl_table_rownode_index($rownodes, $column_mapping['RM'])->textContent;
	      $timestuff = school_crawl_table_rownode_index($rownodes, $column_mapping['TITLE'])->textContent;
	      /*
	       * 2:00 PM - 5:00 PM, W
	       * 1:00 PM - 1:50 PM, M/W/F
	       * 2:00 PM - 5:00 PM, T
	       * 1:50 PM - 4:50 PM, Th
	       * 8:00 AM - 9:15 AM, T/Th
	       */
	      if (!preg_match('/(\\d*:\\d* .M) - (\\d*:\\d* .M), (.*)/', $timestuff, $matches))
		{
		  school_crawl_logf($school_crawl_log, 7, "Unable to parse timestuff: %s",
				    $timestuff);
		  continue;
		}
	      $time_start = school_crawl_time_format(strptime($matches[1], '%l:%M %p'));
	      $time_end = school_crawl_time_format(strptime($matches[2], '%l:%M %p'));
	      $days = school_crawl_days_format($school_crawl_log, explode('/', $matches[3]));

	      school_crawl_logf($school_crawl_log, 1, "Adding %s", $dept . '-' . $course . '-' . $section);
	      $semester->section_add($dept, $course, new Section($section, array(new SectionMeeting($days, $time_start, $time_end, $room, 'lecture', $instructor)), NULL, $credits), $title);

	      /* Save labs for dependency generation later */
	      if (preg_match('/L$/', $course))
		{
		  $labs += array($dept => array());
		  $labs[$dept][$course] = TRUE;
		}
	    }
	}
    }

  /* Bind lab dependencies */
  foreach ($labs as $dept => $courses)
    {
      foreach ($courses as $course => $true)
	{
	  $course_base = $semester->class_get($dept, substr($course, 0, strlen($course) - 1));
	  if (!empty($course_base))
	    $course_base->dependency_add($semester->class_get($dept, $course));
	}
    }
}
