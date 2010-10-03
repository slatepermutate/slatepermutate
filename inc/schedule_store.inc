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

require_once('class.schedule.php');

/**
 * \brief
 *   Initialize a schedule_store.
 *
 * \param $dir
 *   Directory to use as the schedule storage.
 * \return
 *   A schedule_store handle or NULL on failure.
 */
function schedule_store_init($dir = 'saved_schedules')
{
  $schedule_store = array();

  if (!is_dir($dir) || !is_writable($dir))
    {
      error_log('I can\'t write to ' . $dir . ' or it is not a directory!');
      return NULL;
    }

  $schedule_store['dir'] = realpath($dir);

  return $schedule_store;
}

/**
 * \brief
 *   Store a saved schedule into the schedule storage.
 *
 * \param $schedule_store
 *   The schedule_store handle from schedule_store_init().
 * \param $schedule
 *   The schedule object of type Schedule.
 * \return
 *   The newly-saved schedules global ID (numeric) or NULL on error.
 */
function schedule_store_store($schedule_store, $schedule)
{
  $tempfile_name = tempnam($schedule_store['dir'], 'sch');

  $new_schedule_id_name = tempnam($schedule_store['dir'], 'id');
  $new_schedule_id_file = fopen($new_schedule_id_name, 'wb');

  _schedule_store_flock_grab($schedule_store, LOCK_EX);
  /* if the file doesn't exist, we'll end up with a value of 1 for our first entry. */
  $schedule_id = (int)file_get_contents($schedule_store['dir'] . DIRECTORY_SEPARATOR . 'lastid');
  $new_schedule_id = $schedule_id + 1;
  fwrite($new_schedule_id_file, $new_schedule_id);
  fclose($new_schedule_id_file);
  rename($new_schedule_id_name, $schedule_store['dir'] . DIRECTORY_SEPARATOR . 'lastid');
  _schedule_store_flock_release($schedule_store);

  /* we need to serialize the schedule _after_ giving it an ID */
  $schedule->id_set($new_schedule_id);
  file_put_contents($tempfile_name, serialize($schedule));

  rename($tempfile_name, $schedule_store['dir'] . DIRECTORY_SEPARATOR . $new_schedule_id);

  return $new_schedule_id;
}

/**
 * \brief
 *   Retrieve a stored saved schedule from the schedule storage.
 *
 * \param $schedule_store
 *   The schedule_store handle from which to retrieve the saved
 *   schedule.
 * \param $schedule_id
 *   The saved schedule's globally-accessible ID. This value must have
 *   been returned from schedule_store_store() at one point.
 * \return
 *   A Schedule object whose ID was $schedule_id or NULL if
 *   $schedule_id is an invalid or not-yet-allocated schedule
 *   identifier.
 */
function schedule_store_retrieve($schedule_store, $schedule_id)
{
  if (strcmp($schedule_id, (int)$schedule_id))
    return NULL;
  $schedule_id = (int)$schedule_id;

  $schedule_serialized = file_get_contents($schedule_store['dir'] . DIRECTORY_SEPARATOR . $schedule_id);
  if ($schedule_serialized === FALSE)
    return NULL;

  $schedule = unserialize($schedule_serialized);
  if ($schedule === FALSE)
    return NULL;
  return $schedule;
}

/**
 * \brief
 *   Delete a saved schedule.
 *
 * \param $schedule_store
 *   The store from which to delete the schedule.
 * \param $schedule_id
 *   The identifier of the schedule to delete.
 */
function schedule_store_delete($schedule_store, $schedule_id)
{
  $remove_filename = $schedule_store['dir'] . DIRECTORY_SEPARATOR . $schedule_id;
  /* avoid an E_WARNING if the file doesn't exist */
  if (file_exists($remove_filename))
    unlink($remove_filename);
}

/**
 * \brief
 *   Obtains a lock on the /lastid file in the schedule_store.
 *
 * \see _schedule_store_flock_release().
 *
 * \param $schedule_store
 *   The schedule_store instance we're working with.
 * \param $operation
 *   Which flock() operation to perform: valid are LOCK_SH and LOCK_EX.
 */
function _schedule_store_flock_grab(&$schedule_store, $operation)
{
  $schedule_store['lastid_flock_file'] = fopen($schedule_store['dir'] . DIRECTORY_SEPARATOR . 'lastid.flock', 'c');
  return flock($schedule_store['lastid_flock_file'], $operation);
}

/**
 * \brief
 *   Release a lock grabbed with _schedule_store_flock_grab().
 */
function _schedule_store_flock_release(&$schedule_store)
{
  flock($schedule_store['lastid_flock_file'], LOCK_UN);
  fclose($schedule_store['lastid_flock_file']);
  unset($schedule_store['lastid_flock_file']);
}
