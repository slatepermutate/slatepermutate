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

function umich_info()
{
  return array('name' => 'University of Michigan',
               'url' => 'http://umich.edu/',
               'domains' => array(
                                  'umich.edu',
                                  ),
               'student_address' => 'Wolverine',
               );
}

function umich_instructions_html()
{
  return <<<EOF
<h2>U of M-specific Instructions</h2>
<p>
  SlatePermutate can be a useful tool for scheduling your next semester.
</p>
<ol>
  <li>Get in touch with your advisor during advising/reading recess.</li>
  <li>Look up each class your advisor specified on this course listing page</li>
  <li>Enter each class into a SlatePermutate schedule and add each section that is listed that you are willing to take.</li>
  <li>Submit your schedule and view all of the different permutations of your schedule which would work with the sections you specified.</li>
  <li>Print out your preferred schedule by choosing "print" and selecting a schedule.</li>
  <li>Wait until it's your turn to register and grab your preferred sections before they fill up!</li>
</ol> <!-- ' -->
EOF;
}

/** Filter out whitepace items */
function umich_arrayfilter_callback($item){
    if(ltrim($item) == ''){
      return false;
    }
    else{
      return true;
    }
}

/** Parse html at URL into array, first row is row headers */
function umich_table_parse($url) {
  $arr = array();
  $dom = new DOMDocument;
  $html = file_get_contents($url);
  if(!$html){
    return 1;
  }
  $dom->loadHTML($html);
  $dom->preserveWhiteSpace = false;
  $tables = $dom->getElementsByTagName('table');
  $rows = $tables->item(3)->getElementsByTagName('tr'); // Get first table on page 
  foreach ($rows as $rownum => $row) {
    if($rownum > 5) {
      $cols = $row->getElementsByTagName('td');
      foreach($cols as $colnum => $col){
        $arr[$rownum][$colnum] = $col->nodeValue;
      }
    }
  }
  foreach($arr as &$item) {
    $item = array_filter($item, "umich_arrayfilter_callback");
  }

  $arr = array_values($arr); // Reindex array
 
  // Strip navigation and trailing garbage
  $arr[count($arr)-3] = NULL;
  $arr[count($arr)-2] = NULL;
  $arr[count($arr)-1] = NULL;

  $arr = array_filter($arr);
  return $arr;
}

/** Crawls uMich course listings. $season is "f" or "s", year is 2-digit year */
function umich_crawl($semester)
{
  $year = substr($semester->year_get(), 2);
  $season = strtolower(substr($semester->season_get(), 0, 1));

  /* Current academic departments. Update as needed. */
  $departments = array('AAPTIS','ACABS','AERO','AEROSP','AMCULT','ANTHRARC','ANTHRBIO','ANTHRCUL','AOSS','APPPHYS','ARCH','ARMENIAN','ARTDES','ASIAN','ASIANLAN','ASTRO','AUTO','BCS','BIOINF','BIOLCHEM','BIOLOGY','BIOMEDE','BIOPHYS','CAAS','CEE','CHE','CHEM','CIC','CICS','CJS','CLARCH','CLCIV','CMPLXSYS','COMM','COMP','COMPLIT','CSP','CZECH','DANCE','DUTCH','ECON','EDCURINS','EDUC','EEB','EECS','ELI','ENGLISH','ENGR','ENSCEN','ENVIRON','ESENG','FRENCH','GEOG','GEOSCI','GERMAN','GREEK','GTBOOKS','HBEHED','HISTART','HISTORY','HJCS','HMP','HONORS','INTMED','IOE','ITALIAN','JAZZ','JUDAIC','KINESLGY','LACS','LATIN','LHC','LHSP','LING','MACROMOL','MATH','MATSCIE','MCDB','MECHENG','MEDADM','MEDCHEM','MEMS','MENAS','MFG','MICROBIOL','MILSCI','MKT','MODGREEK','MOVESCI','MUSEUMS','MUSICOL','MUSMETH','MUSTHTRE','NAVARCH','NAVSCI','NERS','NEUROSCI','NRE','NURS','OMS','ORGSTUDY','PAT','PATH','PHARMACY','PHIL','PHRMACOL','PHYSICS','PHYSIOL','POLISH','POLSCI','PORTUG','PSYCH','PUBHLTH','PUBPOL','RCARTS','RCCORE','RCHUMS','RCIDIV','RCLANG','RCNSCI','RCSSCI','REEES','RELIGION','ROMLANG','ROMLING','RUSSIAN','SAC','SAS','SCAND','SEAS','SI','SLAVIC','SOC','SPANISH','STATS','STDABRD','SWC','TCHNCLCM','THEORY','THTREMUS','UC','UKRAINE','UP','WOMENSTD','YIDDISH');

  $basepath = "http://www.lsa.umich.edu/cg/cg_results.aspx";
  $yearsyn = 1800 + $year; // Weird year synonym name where 2000 == 1800
  $basepath .= "?termArray={$season}_{$year}_${yearsyn}&cgtype=ug";
  $season = strtolower($season);
  $tables = array();
  foreach($departments as $department) {
   $tables[$department] = umich_table_parse($basepath . '&department=' . $department . '&allsections=true&show=1000');
  }
  return $tables;
}

