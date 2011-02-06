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

/**
 * \brief
 *   Represent a time/location that a Section meets during/at.
 *
 * A Calvin student might ask ``Why is there a need for a
 * SectionMeeting class? Doesn't every Section have a unique
 * prof/time/location?'' A Cedarville student would retort ``Huh?
 * Don't some of your classes have labs in addition to lecture? How do
 * you know that you have to go to both a lecture and lab -- and how
 * do you handle that some classes have different lecture times for
 * different days of the week?''. A Calvin section _is_ a unique
 * prof/time/location. At Cedarville, a Section refers to a prof and a
 * _set_ of time/location pairs. The generalization is to make each
 * Section support a list of meeting times/locations.
 */
class SectionMeeting
{
  private $time_start;
  private $time_end;
  private $days;
  private $location;

  /**
   * \brief
   *   Construct a SectionMeeting.
   *
   * \param $days
   *   A string of single-char day upon which a section meets. Monday
   *   is represented with 'm', Tuesday with 't', Wednesday with 'w',
   *   Thursday with 'h', and Friday with 'f'.
   * \param $time_start
   *   The time of day when the section meeting starts.
   * \param $time_end
   *   The time of day when the section meeting ends.
   * \param $location
   *   Where the meeting occurs. Often a room number of some sort.
   * \param $type
   *   The type of meeting this is. For lectures, please use
   *   'lecture'. For labs, please use 'lab'. For others, use the
   *   school's notation.
   */
  public function __construct($days, $time_start, $time_end, $location = NULL, $type = 'lecture')
  {
    $this->days_set($days);

    $this->time_start = $time_start;
    $this->time_end = $time_end;

    $this->location = $location;

    $this->type = $type;
  }

  /**
   * \brief
   *   Take a days of week string and store it into our $days of week array.
   *
   * \param $days_str
   *   The days of the week in a string format. One char per
   *   day. Mon-Fri is represented with 'm', 't', 'w', 'h', 'f'.
   */
  private function days_set($days_str)
  {
    $this->days = array(0 => FALSE, 1 => FALSE, 2 => FALSE, 3 => FALSE, 4 => FALSE, 5 => FALSE);

    $days_str_strlen = strlen($days_str);
    for ($i = 0; $i < $days_str_strlen; $i ++)
      $this->days[self::day_atoi($days_str[$i])] = TRUE;
  }

  /**
   * \brief
   *   Convert a day letter to a day numeral.
   *
   * Works fine if you give the numeral as well.
   */
  private static function day_atoi($day_c)
  {
    static $day_atoi = array('m' => 0, 't' => 1, 'w' => 2, 'h' => 3, 'f' => 4, 's' => 5,
			     'M' => 0, 'T' => 1, 'W' => 2, 'H' => 3, 'F' => 4, 'S' => 5,
			      0  => 0,  1  => 1,  2  => 2,  3  => 3,  4  => 4,  5  => 5);

    return $day_atoi[$day_c];
  }

  /**
   * \brief
   *   Determine whether or not this class meeting time meets on a
   *   specified day of the week or not.
   *
   * \param $day
   *   The letter or numeral of the day to retrieve.
   * \return
   *   TRUE if this section meeting meets on that day. FALSE
   *   otherwise.
   */
  public function getDay($day)
  {
    return $this->days[self::day_atoi($day)];
  }

  /**
   * \return
   *   This SectionMeeting's location or NULL if none is defined.
   */
  public function getLocation()
  {
    return $this->location;
  }

  /**
   * \brief
   *   Get the start_time of this meeting in slate_permutate's internal format.
   */
  public function getStartTime()
  {
    return $this->time_start;
  }

  public function getEndTime()
  {
    return $this->time_end;
  }

  /**
   * \brief
   *   Get the type of section meeting this is.
   *
   * Examples of Section meeting types include 'lecture' and
   * 'lab'. Currently, any string may be used as a Section meeting
   * type; the possibilities for this field are controlled by the
   * crawl scripts authors' choices about what they pass to
   * SectionMeeting().
   *
   * \return
   *   A string indicating the type of section meeting.
   */
  public function type_get()
  {
    return $this->type;
  }

  /**
   * \brief
   *   Check if this section conflicts with the given section.
   *
   * \param $that
   *   The other section for which I should check for conflicts.
   * \return
   *   TRUE if there is a conflict, FALSE otherwise.
   */
  public function conflictsWith(SectionMeeting $that)
  {
    /*
     * The two sections meetings can't conflict if the start/end times
     * don't overlap. Also, use >= or <= here so that one can say ``I
     * have gym from 10 through 11 and then latin from 11 though 12''.
     */	
    if ($this->getStartTime() >= $that->getEndTime()
	|| $this->getEndTime() <= $that->getStartTime())
      {
	return FALSE;
      }

    /*
     * Now we know that the sections meetings overlap in start/end
     * times. But if they don't both meet on the same day at least
     * once, they don't conflict.
     */
    for ($day = 0; $day < 6; $day ++)
      {
	if ($this->getDay($day) && $that->getDay($day))
	  return TRUE;
      }

    /*
     * The sections meetings don't both share a day of the week.
     */
    return FALSE;
  }

  /**
   * \brief
   *   Return an array of JSON keys specific to this section meeting
   *   time.
   *
   * Currently, the AJAX UI doesn't recognize that a given section may
   * have multiple meeting times. Thus, we simulate this by having
   * multiple instances of the same section but just with different
   * times in the UI.
   */
  public function to_json_array()
  {
    static $daymap = array(0 => 'm', 1 => 't', 2 => 'w', 3 => 'h', 4 => 'f', 5 => 's');

    $json_array = array(
			'time_start' => $this->time_start,
			'time_end' => $this->time_end,
			'days' => array(),
			'location' => $this->location,
			'type' => $this->type,
			);

    for ($day = 0; $day < 6; $day ++)
      $json_array['days'][$daymap[$day]] = $this->getDay($day);

    return $json_array;
  }
}
