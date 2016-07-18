#!/usr/bin/env php-cgi
<?php /* -*- mode: php; -*- */

  include_once 'inc/class.graph.php';

  // Make array of values
  $arr = array();

  $startDate = strtotime("-1 month");
  $stopDate = time(); // now
  $dir = 'saved_schedules/';
  // Do this the new fun php5 OO-way
  foreach(new DirectoryIterator($dir) as $key => $file) {
    if(is_numeric($file->getFilename())){
      $uCtime = $file->getCTime();
      $strCtime = date("m/d/Y",$uCtime);
      $ctime = strtotime($strCtime); // Results in a day-specific unix timestamp

      if($ctime < $stopDate && $ctime > $startDate) {
        if(!isset($arr[$ctime])) {
          $arr[$ctime] = 1;
        }
        else { 
          $arr[$ctime]++;
        }
      }
    }
  }


  $gphArr = array();
  $i = 0;
  foreach($arr as $index => $item) {
    $gphArr[$i]['count'] = $item;
    $gphArr[$i]['label'] = date("n/j", $index);
    $i++;
  }
/*
echo "<pre>";
  print_r($arr);
  print_r($gphArr);  */
  // Graph array
  $myGraph = new barGraph($gphArr, 900, 100);
