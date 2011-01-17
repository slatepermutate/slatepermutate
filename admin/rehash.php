#!/usr/bin/env php
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
 *
 * Runs through schools.d grabbing and caching data, such as the
 * school listing used for the ``choose your school list''.
 */

$inc_base = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;
require_once($inc_base . 'school.inc');
require_once($inc_base . 'school.crawl.inc');
require_once($inc_base . 'class.semester.inc');
require_once($inc_base . 'admin.inc');

return main($argc, $argv);

function main($argc, $argv)
{
  $n = test();
  if ($n)
    {
      fprintf(STDERR, "%d tests failed; exiting\n",
	      $n);
      return 1;
    }

  $opts = getopt('hV:', array('no-crawl', 'crawl-only:', 'help', 'verbosity:'));

  if (isset($opts['help']) || isset($opts['h']))
    {
      usage($argv[0]);
      return 0;
    }

  $crawl = TRUE;
  if (isset($opts['no-crawl']))
    $crawl = FALSE;
  if (isset($opts['crawl-only']))
    $crawl_only = split(',', $opts['crawl-only']);

  $verbosity = 1;
  if (isset($opts['verbosity']))
    $verbosity = (int)$opts['verbosity'];
  if (isset($opts['V']))
    $verbosity = (int)$opts['V'];
  if ($verbosity < 0 || $verbosity > 10)
    {
      fprintf(STDERR, "error: Invalid verbosity level: %d\n", $verbosity);
      fprintf(STDERR, "\n");
      usage();
      return 1;
    }

  if ($crawl)
    {
      $ret = school_cache_recreate($crawl_only, $verbosity);
      if ($ret)
	{
	  fprintf(STDERR, "error: Unable to successfully crawl schools.\n");
	  return 1;
	}
      else
	{
	  fprintf(STDERR, "Crawling successful.\n");
	}
    }

  return 0;
}


/**
 * \brief
 *   Display CLI interface usage.
 */
function usage($progname)
{
  fprintf(STDERR, "Usage: %s [--no-crawl] [--crawl-only=<school_id1>,<school_id2>,...] [--help] [-h]\n"
	  . "\n"
	  . " -h, --help   Show this usage information and exit.\n"
	  . "\n"
	  . " --no-crawl   Disable crawling during this rehash but preserve\n"
	  . "              previous cached crawl data.\n"
	  . " --crawl-only Takes a comma-separated list of school_ids whose\n"
	  . "              registration systems should be crawled for autofill\n"
	  . "              data. Cached data from schools not listed is preserved\n"
	  . " -v, --verbosity Set the verbosity level. Valid range is from 0\n"
	  . "              through 10.",
	  $progname);
}

