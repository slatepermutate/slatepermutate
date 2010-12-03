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

/**
 * \brief
 *   Simulate some aspects of a web browser while retreiving a
 *   document.
 *
 * This allows us to view our cookies in an associative array and to
 * have the server's response automatically update our cookies.
 *
 * If $post is specified as an associative array, an HTTP POST is
 * performed and the data is encoded properly as if we were performing
 * a form submission.
 *
 * Follows redirects. If there is a redirect, the page from which you
 * are redirected is lost... but few people put any information on
 * those pages anyways ;-).
 *
 * \param $uri
 *   The URL to fetch. If a redirect occurs, this is updated.
 * \param $cookies
 *   An associative array of cookies and where to save new cookies.
 * \param $post
 *   If not NULL, causes an HTTP POST. In that case, should be an
 *   associative array of form keys/values.
 * \param $verbosity
 *   How verbose to be.
 * \param $loopspin
 *   An internal variable to prevent us from following perpetual
 *   redirects.
 * \return
 *   The body of the document returned by the server (normally
 *   malformed HTML, especially with Calvin's WebAdvisor
 *   installation).
 */
function school_crawl_geturi(&$uri, &$cookies, $post = NULL, $verbosity = 0, $loopspin = 0)
{
  global $school_crawl_geturi_write_buf, $school_crawl_geturi_headers_buf, $school_crawl_geturi_verbosity;

  if ($verbosity > 5)
    {
      echo "\n";
      echo 'school_crawl_geturi(' . $uri . ")\n";
      echo "\n";
    }

  $curl = curl_init();

  $school_crawl_geturi_verbosity = $verbosity;
  $school_crawl_geturi_write_buf = '';
  $school_crawl_geturi_headers_buf = '';
  curl_setopt($curl, CURLOPT_URL, $uri);

  $cookies_str = '';
  foreach ($cookies as $key => $val)
    {
      if (strlen($cookies_str))
	$cookies_str .= ';';
      $cookies_str .= $key . '=' . $val;
    }

  if ($verbosity > 8)
    echo 'cookies sent: ' . $cookies_str . "\n";
  curl_setopt($curl, CURLOPT_COOKIE, $cookies_str);
  curl_setopt($curl, CURLOPT_HEADERFUNCTION, 'school_crawl_geturi_header_cb');
  curl_setopt($curl, CURLOPT_WRITEFUNCTION, 'school_crawl_geturi_write_cb');

  if ($post != NULL && is_array($post))
    {

      /* var_dump($post); */

      $posttxt = '';
      foreach ($post as $postkey => $postval)
	{
	  $posttxt .= (strlen($posttxt) ? '&' : '')
	    . urlencode($postkey) . '=' . (strpos($postkey, 'MEMBER') === FALSE ? urlencode($postval) : $postval);
	}
      if ($verbosity > 8)
	echo 'setting POST to ' . $posttxt . "\n";

      /* curl_setopt($curl, CURLOPT_POST, TRUE); */
      curl_setopt($curl, CURLOPT_POSTFIELDS, $posttxt);
    }

  curl_exec($curl);
  curl_close($curl);

  $location = NULL;
  foreach (explode("\r\n", $school_crawl_geturi_headers_buf) as $header)
    {
      /*
       * yes, we don't want the line if the first char is a ':' or if it has no ':'
       */
      if (!strpos($header, ':'))
	continue;
      list($header_name, $header_val) = explode(': ', $header, 2);

      if ($verbosity > 8)
	echo $header_name . ' : ' . $header_val . "\n";

      switch($header_name)
	{
	case 'Set-Cookie':
	  list($cookie_name, $cookie_val) = explode('=', $header_val, 2);
	  if ($verbosity > 9)
	    {
	      if (isset($cookies[$cookie_name]))
		echo 'Replacing cookie ' . $cookie_name . '=' . $cookies[$cookie_name]
		  . ' with ';
	      echo 'new cookie ' . $cookie_name . '=' . $cookie_val . "\n";
	    }
	  $cookies[$cookie_name] = $cookie_val;
	  break;

	case 'Location':
	  $location = $header_val;
	  $location = preg_replace(';(kvdata\.calvin\.edu/)(WebAdvisor);', '\1walive/\2', $location) . "\n";
	  $post = NULL;
	  break;
	}
    }

  if ($verbosity > 9)
    echo $school_crawl_geturi_write_buf;
  if ($location && $loopspin < 6)
    {
      $uri = $location;
      return school_crawl_geturi($uri, $cookies, $post, $loopspin + 1);
    }
  return $school_crawl_geturi_write_buf;
}

function school_crawl_geturi_header_cb($curl, $header_buf)
{
  global $school_crawl_geturi_headers_buf;
  $school_crawl_geturi_headers_buf .= $header_buf;
  return strlen($header_buf);
}

function school_crawl_geturi_write_cb($curl, $write_buf)
{
  global $school_crawl_geturi_write_buf;
  $school_crawl_geturi_write_buf .= $write_buf;
  return strlen($write_buf);
}
