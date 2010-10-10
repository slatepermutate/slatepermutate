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
