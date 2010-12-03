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

  include_once 'inc/class.page.php';
  $scripts = array('jQuery','jQueryUI'); 
  $adminpage = new page('Administration',$scripts);

  if(!isset($admin_pass)) {
    echo "<p>Administration password not configured. See config.inc for more information.</p>";
    $adminpage->foot();
    exit;
  }

  // Force authentication
  else if (!isset($_SERVER['PHP_AUTH_USER']) || (!isset($_SERVER['PHP_AUTH_PW'])) || $_SERVER['PHP_AUTH_PW'] != $admin_pass) {
    header('WWW-Authenticate: Basic realm="My Realm"');
    header('HTTP/1.0 401 Unauthorized');
    echo '<p>You must authenticate to view this page.</p>';
    $adminpage->foot();
    exit;
  }

  function isBeforeDate($first, $reference) {
   return true; 
  }

  function emptySavedDir($todate = null) {
    // Empty the saved_schedules directory
    $dir = "saved_schedules";
    if(!is_dir($dir)) {
      echo "<p><pre>{$dir}</pre> is not a valid directory!";
      return;
    }
   
    // Do this the new fun php5 OO-way
    foreach(new DirectoryIterator($dir) as $file) {
      $isBeforeDate = isBeforeDate($file->getCTime, $todate);
      if(!$todate || $isBeforeDate) {
        if(is_numeric($file->getFilename())){
          // unlink($dir . '/' . $file->getFilename());
          echo "<p>Erased file: " . $dir . '/' . $file->getFilename() . "</p>";
        } 
      }
    }
  }

  function checkAction() {
    $result = '';
    if(isset($_GET['rehash'])) {
      // Run the rehash
      $result = 'Rehash Complete'; 
    }
    else if(isset($_GET['purgetodate'])) {
      // Purge saved schedule cache up to date
      emptySavedDir($_GET['purgetodate']);
      $result = 'Purged all saved schedules up to ' . $_GET['purgetodate'];
    }
    else if(isset($_GET['purge'])) {
      // Purge the saved schedule cache
      emptySavedDir();
      $result = 'Purge Complete';
    }
    return $result;
  }

  function getLastRehash(){
    $stats = stat("cache/schools");
    if(!$stats){
      return "never";
    }
    return date("F j, Y, g:i a", $stats[9]);
  }

  function getSchools() {
    if(!stat("cache/schools")){
      return false;
    }

    $schoolsArr = unserialize(file_get_contents("cache/schools"));
    return $schoolsArr;
  }

  function schoolsDropList(){
    $schools = getSchools();
    echo '<select>';
    foreach($schools['list'] as $school){
      if(!$school['name'] != "Generic College") {
        echo '<option ';
        if(!$school['crawled']) {
          echo 'class="bold" ';
        }
        echo 'value="' . $school['name'] . '">';
        echo $school['name'];
        
        
        echo "</option>";
      }
    }
    echo "</select>";
  }

  function getNumSaved(){
    return file_get_contents("saved_schedules/lastid");
  }

?>


<?php /* Check if authorized */ 

  $res = checkAction();
  if($res != '') {
    echo '<p><em>' . $res . '</em> <a href="admin.php">(x)</a></p>';
  }
?>

<!-- Move to pagegen class instantiation -->
<script>
	$(function() {
		$( "#datepicker" ).datepicker();
                $( "#datepicker" ).datepicker( "option", "dateFormat", "yy-mm-dd" );

	});
</script>


<h3>Update</h3>
<p>You are currently running version <?php echo SP_PACKAGE_VERSION ?>. The latest available release is VERSION.</p>

<h3>Rehash</h3>
<p>Last full rehash ocurred on  <?php echo getLastRehash() ?>.</p>
<ul>
  <li><a href="admin.php?rehash">Rehash All Institutions</a></li>
  <li><?php schoolsDropList(); ?> <a href="admin.php?rehash">Rehash Institution</a> </li>
</ul>

<h3>Purge</h3>
<p>The cache currently holds <?php echo getNumSaved(); ?> schedules.</p>
<ul>
  <li><a href="admin.php?purge">Purge Entire Cache</a></li>
  <li><form action="admin.php">Purge cache up to <input type="text" name="purgetodate" size="8" id="datepicker"/><input type="submit" value="Go&raquo;" /></form></li>
</ul>

<?php
$adminpage->foot();
