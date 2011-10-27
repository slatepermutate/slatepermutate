<?php /* -*- mode: php; indent-tabs-mode: nil; -*- */
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

if (!function_exists('mean'))
  {
    /**
     * \brief
     *   Calculate the mean of a set of numerical values without
     *   overflowing stuff.
     */
    function mean(array $values)
    {
      /*
       * As the influence of each element reduces with each iteration
       * in the used algorithm, shuffling the array should give a
       * better idea of what the actual mean is for larger arrays.
       */
      shuffle($values);

      $val = 0;
      $i = 0;
      foreach ($values as $value)
        {
          $val = $val * $i / ($i + 1)
            + $value / ($i + 1);
          $i ++;
        }

      return $val;
    }
  }

if (!function_exists('sum'))
  {
    /**
     * \brief
     *   Add all elements in a set together.
     *
     * \parram $S
     *   The set to sum up.
     * \return
     *   The sum of all elements in the set.
     */
    function sum($S)
    {
      $ret = 0;
      foreach ($S as $S_i)
        $ret += $S_i;
      return $ret;
    }
  }

if (!function_exists('stddev'))
  {
    function stddev(array $values)
    {
      $mean = mean($values);

      $squares = 0;
      $i = 0;
      foreach ($values as $value)
        $squares += pow($mean - $value, 2);
      return sqrt($squares / (count($values) - 1));
    }
  }

/**
 * \brief
 *   Return the four quartile points of an array of sorted values with
 *   normal integral indexes.
 */
function sp_iqr(array $values)
{
  $count = count($values);
  if (!$count)
    return array(0, 0, 0, 0);
  return array(
               $values[0],
               $values[(int)($count / 4)],
               $values[(int)($count / 2)],
               $values[(int)(3 * $count / 4)],
               $values[$count - 1],
               );
}

/**
 * \brief
 *   Remove any `outliers' from an array of values.
 *
 * An outlier is defined as any value that falls further than 1.5
 * standard deviations outside of some sort of inter-quartile range.
 */
function filter_outliers(array $values)
{
  sort($values, SORT_NUMERIC);
  $values = array_values($values);

  $stddev = stddev($values);
  list(, $iqr_min, $iqr_max, ) = sp_iqr($values);

  $min = $iqr_min - 1.5 * $stddev;
  $max = $iqr_max + 1.5 * $stddev;

  $count = count($values);
  for ($i = 0; $i < $count; $i ++)
    if ($values[$i] < $min
        || $values[$i] > $max)
      unset($values[$i]);
  return array_values($values);
}
