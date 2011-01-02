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
 * Provides a way to easily purge a number of saved schedules through
 * a date range.
 */

$inc_base = dirname(__FILE__) . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR;
require_once($inc_base . 'admin.inc');

return main($argc, $argv);

function main($argc, $argv)
{
  global $admin_enable_purge;

  $n = test();
  if ($n)
    {
      fprintf(STDERR, "%d tests failed; exiting\n",
	      $n);
      return 1;
    }

  $opts = getopt('hV:', array('all', 'max-time:', 'min-time:', 'help', 'verbosity:'));

  if (isset($opts['help']) || isset($opts['h']))
    {
      usage($argv[0]);
      return 0;
    }

  $all = FALSE;
  if (isset($opts['all']))
    $crawl = TRUE;

  foreach (array('min', 'max') as $time_var)
    {
      if (isset($opts[$time_var . '-time']))
	{
	  ${$time_var . '_time'} = strtotime($opts[$time_var . '-time']);
	  /* two ways of saying the thing failed -- was -1 until PHP 5.1.0 */
	  if (${$time_var . '_time'} === FALSE || ${$time_var . '_time'} == -1)
	    {
	      fprintf(STDERR, "%s-time: Invalid date/time format, can't understand string: ``%s''.\n",
		      $time_var, $opts['min-time']);
	      return 1;
	    }
	}
    }

  if ($all && (isset($min_time) || isset($max_time)))
    {
      fprintf(STDERR, "error: Both --all and one of --max-time or --min-time were specified. These options are mutually exclusive.\n");
      return 1;
    }

  if (!$all && (!isset($min_time) || !isset($max_time)))
    {
      if (isset($min_time) || isset($max_time))
	fprintf(STDERR, "error: If --min-time is set, --max-time must also be set and vice versa. This is to prevent accidents.\n");
      else
	fprintf(STDERR, "error: Either --all or both of --min-time and --max-time must be passed to purge saved schedules.\n");
      return 1;
    }

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

  if ($all)
    {
      fprintf(STDOUT, "Purging all schedules.\n");
    }

  if (!isset($min_time))
    $min_time = 0;
  if (!isset($max_time))
    $max_time = NULL;
  else
    fprintf(STDOUT, "Purging from %s to %s.\n",
	    strftime("%F %T", $min_time), strftime("%F %T", $max_time));

  $schedule_store = schedule_store_init();
  if (!$schedule_store)
    {
      fprintf(STDERR, "error: Unable to initialize schedule_store.\n");
      return 1;
    }
  $ret = schedule_store_purge_range($schedule_store, $min_time, $max_time);
  if ($ret === FALSE)
    {
      fprintf(STDERR, "error: Unable to purge schedule_store%s.\n",
	      $all ? '' : ' range');
      if (!$admin_enable_purge)
	fprintf(STDERR, "error: You need to set " . '$admin_enable_purge' . " in config.inc to enable purging schedules through the administration utilities.\n");
      return 1;
    }

  fprintf(STDOUT, "Schedules deleted: %s\n",
	  $ret);
  fprintf(STDOUT, "Purge successful.\n");

  return 0;
}


/**
 * \brief
 *   Display CLI interface usage.
 */
function usage($progname)
{
  fprintf(STDERR, "Usage: %s [--all] [--min-time=<min time>] [--max-time=<max time>] [--help] [-h]\n"
	  . "\n"
	  . " -h, --help   Show this usage information and exit.\n"
	  . "\n"
	  . " --all        Purge all saved schedules.\n"
	  . " --min-time   Takes a date/time string after which a schedule must\n"
	  . "              have been created to be deleted.\n"
	  . " --max-time   Takes a date/time string before which a schedule must\n"
	  . "              be saved to be dleted.\n"
	  . " -v, --verbosity Set the verbosity level. Valid range is from 0\n"
	  . "              through 10.",
	  $progname);
}

