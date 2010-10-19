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

/**
 * \file
 *   Routines that are only useful when crawling schools' websites for
 *   autofill section data.
 */

/**
 * \brief
 *   Parse a simple time string into slate_permutate's time
 *   representation.
 *
 * \param $time
 *   An array compatible with the return value of strptime(). The only
 *   fields we use are 'tm_hour', which is from 0 through 23, and
 *   'tm_min', which may be from 0 through 50.
 */
function school_crawl_time_format($time)
{
  return sprintf('%02d%02d', $time['tm_hour'], $time['tm_min']);
}

/**
 * \brief
 *   Take an array of day names and assemble them into
 *   slate_permutate's internal (weird) representation of a set of
 *   weekdays.
 *
 * This function is intended to make it easy for one to take the
 * output of an explode() call. For example, to decode $days_str =
 * 'Monday, Tuesday, Friday', one would do
 * school_crawl_days_format(explode(', ', $days_str));
 *
 * \param $days
 *   An array of day names. These may be common abbreviations or
 *   truncations (any truncations must be two chars long for
 *   simplicity. One-char representations are supported, however, but
 *   use 'm', 't', 'w', 'h', 'f' to distinguish Thursday and
 *   Tuesday. 'r' may also be used for Thursday.). Case does not
 *   matter.
 * \return
 *   slate_permutate's strange internal days representation.
 */
function school_crawl_days_format($days)
{
  static $daymap_1 = array('m' => 'm', 't' => 't', 'w' => 'w', 'h' => 'h', 'r' => 'h', 'f' => 'f');
  static $daymap_2 = array('th' => 'h');

  $my_days = array();
  foreach ($days as $day)
    {
      $day_orig = $day;
      $day = strtolower(substr(trim($day), 0, 2));

      /*
       * convert from two-char representation to one-char
       * representation.n
       */
      if (strlen($day) > 1)
	{
	  if (isset($daymap_2[$day]))
	    $day = $daymap_2[$day];
	  else
	    $day = substr($day, 0, 1);
	}
      if (isset($daymap_1[$day]))
	$my_days[$daymap_1[$day]] = TRUE;
      else
	error_log('school_crawl_days_format() got invalid day specifier:'
		  . ' `' . $day_orig . '\' => `' . $day . '\'');
    }

  $day_str = '';
  foreach ($my_days as $day_val => $junk)
    $day_str .= $day_val;

  return $day_str;
}

/**
 * \brief
 *   Take a string of day initials and format it.
 *
 * \param $days_str
 *   Example input: 'mwf', 'TR'.
 * \return
 *   Same as school_crawl_days_format()
 */
function school_crawl_days_str_format($days_str)
{
  $day_initials = array();
  for ($i = 0; $i < strlen($days_str); $i ++)
    $day_initials[] = $days_str[$i];

  return school_crawl_days_format($day_initials);
}
