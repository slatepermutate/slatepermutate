<?php

  include_once 'inc/class.graph.php';

  // Make array of values
  $arr = array();

  // @TODO - pull in isBeforeDate from admin library, create isAfterDate (!isBeforeDate)

  $startDate = "0";
  $stopDate = "99999999999999999999";
  $dir = 'saved_schedules/';
  // Do this the new fun php5 OO-way
  foreach(new DirectoryIterator($dir) as $key => $file) {
    if(is_numeric($file->getFilename())){
      $uctime = $file->getCTime();
      $ctime = date("m/d/Y",$uctime);
      $ctime = strtotime($ctime); // Results in a day-specific unix timestamp

      if($ctime < $stopDate && $ctime > $startDate) {
        $currDate = $file->getCTime();
        if(!isset($arr[$currDate])) {
          $arr[$currDate] = 1;
        }
        else { 
          $arr[$currDate]++;
        }
      }
    }
  }

  $gphArr = array();
  $i = 0;
  foreach($arr as $index => $item) {
    $gphArr[$i]['count'] = $item;
    $gphArr[$i]['label'] = date("j/n", $index);
    $i++;
  }
//echo "<pre>";
//  print_r($gphArr); 
  // Graph array
  $myGraph = new barGraph($gphArr, 900, 100);
?>
