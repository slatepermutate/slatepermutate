<?php
function cedarville_info()
{
  return array('name' => 'Cedarville University',
	       'url' => 'http://cedarville.edu/',
	       'domains' => array(
				  'cedarville.edu',
				  ),
	       'student_address' => 'Cedarville Student',
	       );
}

function cedarville_instructions_html()
{
  return <<<EOF
<h2>Cedarville-specific Instructions</h2>
<p>
  SlatePermutate can be a useful tool for scheduling your next semester at <a href="http://cedarville.edu/">Cedarville University</a>.
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
function table_parse($url) {
  $arr = array();
  $dom = new DOMDocument;
  $html = file_get_contents($url);
  if(!$html){
    return 1;
  }
  $dom->loadHTML($html);
  $dom->preserveWhiteSpace = false;
  $tables = $dom->getElementsByTagName('table');
  $rows = $tables->item(0)->getElementsByTagName('tr'); // Get first table on page 
  foreach ($rows as $rownum => $row) {
    $cols = $row->getElementsByTagName('td');
    foreach($cols as $colnum => $col){
      $arr[$rownum][$colnum] = $col->nodeValue;
    }
  }
  return $arr;
}

/** Crawls Cedarville course listings. $season is "fa" or "sp", year is 4-digit year */
function cedarville_crawl($season, $year) {
  /* Current academic departments. Update as needed. */
  $departments = array('be','ba','ca','ed','eg','es','hg','id','ll','ms','mu','ns','ph','py','sm','sw');
  $basepath = "http://cedarville.edu/courses/schedule/";

  $season = strtolower($season);
  $tables = array();
  foreach($departments as $department) {
    $tables[$department] = table_parse($basepath . $year . $season . '_' . $department . '_' . 'all.htm');
  }
  return $tables;
}

