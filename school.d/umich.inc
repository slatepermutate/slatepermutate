<?php
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
<h2>Umich-specific Instructions</h2>
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
</ol>
EOF;
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
  $rows = $tables->item(2)->getElementsByTagName('tr'); // Get first table on page 
  foreach ($rows as $rownum => $row) {
    $cols = $row->getElementsByTagName('td');
    foreach($cols as $colnum => $col){
      $arr[$rownum][$colnum] = $col->nodeValue;
    }
  }
  return $arr;
}

/** Crawls uMich course listings. $season is "f" or "s", year is 2-digit year */
function umich_crawl($season, $year) {
  /* Current academic departments. Update as needed. */
  $departments = array('AAPTIS','ACABS','AERO','AEROSP','AMCULT','ANTHRARC','ANTHRBIO','ANTHRCUL','AOSS','APPPHYS','ARCH','ARMENIAN','ARTDES','ASIAN','ASIANLAN','ASTRO','AUTO','BCS','BIOINF','BIOLCHEM','BIOLOGY','BIOMEDE','BIOPHYS','CAAS','CEE','CHE','CHEM','CIC','CICS','CJS','CLARCH','CLCIV','CMPLXSYS','COMM','COMP','COMPLIT','CSP','CZECH','DANCE','DUTCH','ECON','EDCURINS','EDUC','EEB','EECS','ELI','ENGLISH','ENGR','ENSCEN','ENVIRON','ESENG','FRENCH','GEOG','GEOSCI','GERMAN','GREEK','GTBOOKS','HBEHED','HISTART','HISTORY','HJCS','HMP','HONORS','INTMED','IOE','ITALIAN','JAZZ','JUDAIC','KINESLGY','LACS','LATIN','LHC','LHSP','LING','MACROMOL','MATH','MATSCIE','MCDB','MECHENG','MEDADM','MEDCHEM','MEMS','MENAS','MFG','MICROBIOL','MILSCI','MKT','MODGREEK','MOVESCI','MUSEUMS','MUSICOL','MUSMETH','MUSTHTRE','NAVARCH','NAVSCI','NERS','NEUROSCI','NRE','NURS','OMS','ORGSTUDY','PAT','PATH','PHARMACY','PHIL','PHRMACOL','PHYSICS','PHYSIOL','POLISH','POLSCI','PORTUG','PSYCH','PUBHLTH','PUBPOL','RCARTS','RCCORE','RCHUMS','RCIDIV','RCLANG','RCNSCI','RCSSCI','REEES','RELIGION','ROMLANG','ROMLING','RUSSIAN','SAC','SAS','SCAND','SEAS','SI','SLAVIC','SOC','SPANISH','STATS','STDABRD','SWC','TCHNCLCM','THEORY','THTREMUS','UC','UKRAINE','UP','WOMENSTD','YIDDISH');
  $basepath = "http://www.lsa.umich.edu/cg/cg_results.aspx";
  $yearsyn = 1810 + $year; // Weird year synonym name where 2000 == 1800
  $basepath .= "?termArray={$season}_{$year}_${yearsyn}&cgtype=ug";
  $season = strtolower($season);
  $tables = array();
  foreach($departments as $department) {
    $tables[$department] = umich_table_parse($basepath . '&department=' . $department . '&allsections=true&show=1000');
  }
  return $tables;
}

