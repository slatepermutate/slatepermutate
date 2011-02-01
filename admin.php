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

require_once('inc/class.page.php');
require_once('inc/admin.inc');


  $scripts = array('jQuery','jQueryUI'); 
  $adminpage = new page('Administration',$scripts,FALSE);
  $datepicker = '$(function() {
                   $( "#datepicker" ).datepicker();
                   $( "#datepicker" ).datepicker( "option", "dateFormat", "yy-mm-dd" );
                 });';
  $adminpage->headcode_add('datePicker', $adminpage->script_wrap($datepicker), TRUE);
  $adminpage->head();

  if(!isset($admin_pass) || !isset($admin_enable) || $admin_enable !== true) {
    echo "<p>The administration interface is not enabled or is improperly configured. See config.inc for more information.</p>";
    $adminpage->foot();
    exit;
  }

  // Force authentication
  else if (!isset($_SERVER['PHP_AUTH_USER']) || (!isset($_SERVER['PHP_AUTH_PW'])) || $_SERVER['PHP_AUTH_PW'] != $admin_pass) {
    header('WWW-Authenticate: Basic realm="My Realm"');
    header('HTTP/1.0 401 Unauthorized');
    echo '<p>You must authenticate to access the administration interface.</p>';
    $adminpage->foot();
    exit;
  }

  function isBeforeDate($toCheck, $reference) {
    $formatted = date("Y-m-d", $toCheck);
    $refUnix = strtotime($reference);

    if($toCheck < $refUnix) {
      return true;
    }
    return false; 
  }

  function checkAction() {
    $result = '';
    if(isset($_GET['rehash'])) {
      // Run the rehash

      $crawl_schools = NULL;
      if (isset($_REQUEST['rehash_school']))
	$crawl_schools = array($_REQUEST['rehash_school']);

      ob_start();
      if (school_cache_recreate($crawl_schools))
	$result = 'Rehash Failed';
      else
	$result = 'Rehash Successful';
      $hashresult = nl2br(ob_get_contents());
      ob_end_clean();
      if ($crawl_schools !== NULL)
	$result .= ': ' . implode(', ', $crawl_schools);

      // Prepend rehash output
      $result = $hashresult . '<br />' . $result;
    }
    else if(isset($_GET['purge']))
      {
	$purge_date = NULL;
	if(isset($_GET['purgetodate']))
	  {
	    $t = strptime($_REQUEST['purgetodate'], '%Y-%m-%d');
	    $purge_date = mktime($t['tm_hour'], $t['tm_min'], $t['tm_sec'], $t['tm_mon'], $t['tm_mday'], $t['tm_year'] + 1900);
	  }

	$schedule_store = schedule_store_init();
	if (!$schedule_store)
	  return 'Purging saved schedules failed: unable to initialize schedule_store handle.';

	$num_purged = schedule_store_purge_range($schedule_store, 0, $purge_date);
	if ($num_purged === FALSE)
	  {
	    $result = 'Purging saved scheduled failed.';
	    if (!$admin_enable_purge)
	      $result .= ' Purging is disabled. To enable purging, set $admin_enable_purge = TRUE in config.inc.';
	  }
	else
	  {
	    $result .= 'Purged ' . $num_purged . ' schedules';
	    if ($purge_date !== NULL)
	      $result .= ' up to ' . $purge_date;
	  }
      }
    return $result;
  }

  function getLastRehash(){
    $stats = stat("cache/schools");
    if(!$stats){
      return false;
    }
    return date("F j, Y, g:i a", $stats[9]);
  }

  function schoolsDropList()
  {
    $school_ids = school_list();
    echo '<select name="rehash_school">';
    foreach($school_ids as $school_id)
      {
	$school = school_load($school_id);
	echo '  <option value="' . $school_id . '">' . $school['name'] . '</option>' . "\n";
      }
    echo "</select>";
  }

  function getMaxSaved()
  {
    $schedule_store = schedule_store_init();
    return schedule_store_getmaxid($schedule_store);
  }

?>


<?php /* Check if authorized */ 

  $res = checkAction();
  if($res != '') {
    echo '<p><em>' . $res . '</em> <a href="admin.php">(x)</a></p>';
  }
?>

<h3>Stats</h3>
<img src="statsGraph.php" />

<h3>Update</h3>
<p>You are currently running version <?php echo SP_PACKAGE_VERSION; ?>. The latest available release is VERSION.</p>

<h3>Rehash</h3>
<?php $lastRehash = getLastRehash();
      if($lastRehash) {
        echo "<p>Last rehash ocurred on $lastRehash.</p>";
      }
      else {
        echo "<p>This installation has not been rehashed. Please <a href=\"admin.php?rehash\">rehash now</a> to download school scheduling metadata.</p>";
      }
?>
<ul>
  <li>
    <a href="admin.php?rehash">Rehash All Institutions</a>
  </li>
  <li>
    <form action="admin.php">Rehash schedules for <?php schoolsDropList(); ?>
      <input type="hidden" name="rehash" value="1" />
      <input type="submit" value="Go &raquo;" />
    </form>
  </li>
</ul>

<h3>Purge</h3>
    <p>The highest saved_schedule id is <a href="<?php $max_saved = getMaxSaved(); echo htmlentities(Schedule::url($max_saved)); ?>"><?php echo $max_saved;?></a>.</p>
<ul>
  <li><a href="admin.php?purge">Purge Entire Cache</a></li>
  <li>
    <form action="admin.php">Purge cache up to 
      <input type="text" name="purgetodate" size="8" id="datepicker"/>
      <input type="submit" value="Go &raquo;" />
      <input type="hidden" name="purge" value="1" /> <!-- simplify our server-side code -->
    </form>
  </li>
</ul>

<?php
$adminpage->foot();
