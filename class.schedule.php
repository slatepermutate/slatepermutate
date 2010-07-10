<?php

//**************************************************
// class.schedule.php	Author: Nathan Gelderloos
//
// Represents a schedule of a week. Stores the
// classes that are part of that week and calculates
// all the possible permutations.
//**************************************************

include_once 'errors.php';
include_once 'class.class.php';
include_once 'class.section.php';

class Schedule
{
    private $classStorage;			// Classes array of classes
    private $nclasses;				// Integer number of classes
    private $nPermutations = 0;		// Integer number of real permutations
    private $possiblePermutations;	// Integer number of possible permutations
    private $scheduleName;			// String name of schedule
    private $storage;				// Integer array of valid schedules
    private $title = "SlatePermutate - Scheduler";

	//--------------------------------------------------
	// Creates a schedule with the given name.
	//--------------------------------------------------
	function __construct($n)
	{
        $this->nclasses = 0;
        $this->scheduleName = $n; 
    }

	//--------------------------------------------------
	// Mutators and Accessors
	//--------------------------------------------------
	public function getName()
	{
		return $this->scheduleName;
	}    

	//--------------------------------------------------
	// Adds a new class to the schedule.
	//--------------------------------------------------
	function addClass($n)
    {
        $this->classStorage[$this->nclasses] = new Classes($n);
        $this->nclasses++;
    }

	//--------------------------------------------------
	// Adds a section to the desired class.
	//--------------------------------------------------
    function addSection($n, $l, $s, $e, $d)
    {
        $found = false;
        $counter = 0;
      
        while(!$found && ($counter < $this->nclasses))
        {
			$temp = $this->classStorage[$counter]->getName();
			
			if((strcmp($temp,$n)) == 0)
            {
				$found = true;
            } else {
				$counter++;
			}
		}
		
		if($counter == $this->nclasses)
		{
			echo "Could not find class: " . $n . "<br />";
        } else {
			$p = "unknown prof";
            $this->classStorage[$counter]->addSection($l, $p, $s, $e, $d);
        }
    }

	//--------------------------------------------------
	// Finds all of the possible permutations and stores
	// the results in the storage array.
	//--------------------------------------------------
	function findPossibilities()
	{
        $this->possiblePermutations = 1;
		$position = 0;
		$counter = 0;

        for($i = 0; $i < $this->nclasses; $i++)
        {
			$this->possiblePermutations = $this->possiblePermutations * $this->classStorage[$i]->getnsections();
			$cs[$i] = 0;	// Sets the counter array to all zeroes.
        }
        
		// Checks for conflicts in given classes, stores if none found
        do
        {
			$conflict = false;
			$upCounter = 0;
         
			// Get first class to compare
			while($upCounter < $this->nclasses)
			{
				$downCounter = $this->nclasses-1;
				while($downCounter > $upCounter)
				{
					$start1 = $this->classStorage[$upCounter]->getSection($cs[$upCounter])->getStartTime();
					$end1 = $this->classStorage[$upCounter]->getSection($cs[$upCounter])->getEndTime();
					$start2 = $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getStartTime();
					$end2 = $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getEndTime();
					
					// If the two times overlap, then check if days overlap.
					if(!$conflict && ((($start1 >= $start2) && ($start1 <= $end2)) || (($start2 >= $start1) && ($start2 <= $end1))))
					{
						// Monday
						if(!$conflict && ($this->classStorage[$upCounter]->getSection($cs[$upCounter])->getM() == $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getM()))
						{
							$conflict = true;
						}
						
						// Tuesday
						if(!$conflict && ($this->classStorage[$upCounter]->getSection($cs[$upCounter])->getTu() == $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getTu()))
						{
							$conflict = true;
						}
						
						// Wednesday
						if(!$conflict && ($this->classStorage[$upCounter]->getSection($cs[$upCounter])->getW() == $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getW()))
						{
							$conflict = true;
						}
						
						// Thursday
						if(!$conflict && ($this->classStorage[$upCounter]->getSection($cs[$upCounter])->getTh() == $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getTh()))
						{
							$conflict = true;
						}
						
						// Friday
						if(!$conflict && ($this->classStorage[$upCounter]->getSection($cs[$upCounter])->getF() == $this->classStorage[$downCounter]->getSection($cs[$downCounter])->getF()))
						{
							$conflict = true;
						}
					}
               
					$downCounter--;
				}
				
				$upCounter++;
            }
            
			// Store to storage if no conflict is found.
			if(!$conflict)
			{
				for($i = 0; $i < $this->nclasses; $i++)
				{
					$this->storage[$this->nPermutations][$i] = $cs[$i];
				}
				$this->nPermutations++;
			}
			
			// Increase the counter by one to get the next combination of class sections.
			$cs[$position] = $cs[$position] + 1;
			
			// Check to make sure the counter is still valid.
			$valid = false;
			while(!$valid)
			{
				if($cs[$position] == $this->classStorage[$position]->getnsections())
				{
					$cs[$position] = 0;

					$position++;
					
					// This is for the very last permutation. Even 
					// though the combination is not actually true
					// the larger while loop will end before any 
					// new combinations are performed.
					if($position == $this->nclasses)
					{
						$valid = true;
					} else {
						$cs[$position]++;
					}
				} else {
					$valid = true;
					$position = 0;
				}
            }
            
			$counter++;
		} while($counter < $this->possiblePermutations);
	}
    
	//--------------------------------------------------
	// Prints out the possible permutations in tables.
	//--------------------------------------------------
	function writeoutTables()
	{
		$table = "";
		$header = "";
		$footer = "";
		$M = -1;
		$Tu = -1;
		$W = -1;
		$Th = -1;
		$F = -1;
		$filled = false;
		$time = array(700,730,800,830,900,930,1000,1030,1100,1130,1200,1230,1300,1330,1400,1430,1500,1530,1600,1630,1700,1730,1800,1830,1900,1930,2000,2030,2100,2130, 2200);

		// Reminder:
		// border-style:top right bottom left
		$header .= "<html><head><title>" . $this->title . "</title>\n\n<style type=\"text/css\">".
				"\n.top{\n\tborder-style:solid solid none solid;\nbackground-color:#dddddd;\n}".
				"\n.mid{\n\tborder-style:none solid none solid;\nbackground-color:#dddddd;\n}".
				"\n.end{\n\tborder-style:none solid solid solid;\nbackground-color:#dddddd;\n}".
				"\n.none{\n\tborder-style:none;\n}".
				"\n.single{\n\tborder-style:solid;\n\tbackground-color:#dddddd;\n}".
				"\ntd{\n\ttext-align:center;\nwidth:7em;\n}".
				"\n.time{\n\tborder-style:none none solid none;\n}".
				"\n.day{\n\tborder-style:none none solid solid;\n}".
				
			"\n</style>".

			"\n<script src=\"http://www.google.com/jsapi\"></script>".
			"\n<script type=\"text/javascript\" charset=\"utf-8\">".
			"\n\tgoogle.load(\"jquery\", \"1.3.2\");".
			"\n\tgoogle.load(\"jqueryui\", \"1.7.2\");".
			"\n</script>".

			"\n<link rel=\"stylesheet\" href=\"styles/general.css\" type=\"text/css\" media=\"screen\" charset=\"utf-8\">".

			"\n<link rel=\"stylesheet\" href=\"styles/glider.css\" type=\"text/css\" media=\"screen\" charset=\"utf-8\">".
			"\n<script src=\"scripts/prototype.js\" type=\"text/javascript\" charset=\"utf-8\"></script>". 
			"\n<script src=\"scripts/effects.js\" type=\"text/javascript\" charset=\"utf-8\"></script>".
			"\n<script src=\"scripts/glider.js\" type=\"text/javascript\" charset=\"utf-8\"></script>".
			
			"\n</head><body>".

			"<p>There were a total of " . $this->possiblePermutations . " possible permutations. Only " . $this->nPermutations . " permutations had no class conflicts.</p>".

			"\n\n<div id=\"header\">\n<h1><em>SlatePermutate</em> - Scheduler</h1>\n</div><div id=\"content\">";
			
		$footer .="</div></div><script type=\"text/javascript\" charset=\"utf-8\">". 
			"\n\tvar my_glider = new Glider('my-glider', {duration:0});".
			"\n</script>\n\n</div><div id=\"footer\">\n<h5>&copy; " . Date("Y") . " <a href=\"http://protofusion.org/~nathang/\">Nathan Gelderloos</a><br />".
			"\nwith special thanks to <a href=\"http://ethanzonca.com\">Ethan Zonca</a></h5>\n</div>";

		$footer .="\n\n</body></html>";

		if($this->nPermutations > 0)
		{
			$table .= "<div id=\"my-glider\"><div class=\"controls\">";
			
			for($nn = 1; $nn <= $this->nPermutations; $nn++)
			{
			$table .= "<a href=\"#section" . $nn . "\">&nbsp;" . $nn . "&nbsp;</a>";
			}
			
			$table .= "</div><div class=\"scroller\"><div class=\"scontent\">";
		
			for($i = 0; $i < $this->nPermutations; $i++)
			{
				$table .= "<div class=\"section\" id=\"section" . ($i+1) . "\">";
  
				// Beginning of table
				$table .= "<table style=\"empty-cells:show;\" border=\"1\" cellspacing=\"0\">";
				
				// Header row
				$table .= "\n\t<tr>\n\t\t<td class=\"none\">" . ($i+1) . "</td>\n\t\t<td class=\"day\">Monday</td>\n\t\t<td class=\"day\">Tuesday</td>\n\t\t<td class=\"day\">Wednesday</td>\n\t\t<td class=\"day\">Thursday</td>\n\t\t<td class=\"day\">Friday</td>\n\t</tr>";

				for($r = 0; $r < (count($time)-1); $r++)
				{
					// Beginning of new row
					$temp = $time[$r];
					if($temp > 1259)
					{
						$temp = $temp-1200;
					}

					$table .= "\n\t<tr>\n\t\t<td class=\"time\">" . $this->prettyTime($time[$r]) . "</td>";

					//---------------MONDAY---------------
					for($j = 0; $j < $this->nclasses; $j++)
					{
						if($M == -1)
						{
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getM())
							{
								if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
								{
									if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
									{
										$table .= "\n\t\t<td class=\"top\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$M = $j;
										$filled = true;
									} else {
										$table .= "\n\t\t<td class=\"single\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$filled = true;
									}
								}
							}
						} else {
							if($j == $M)
							{
								if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
								{
									$table .= "\n\t\t<td class=\"mid\">&nbsp;</td>";
									$filled = true;
								} else {
									$table .= "\n\t\t<td class=\"end\">&nbsp;</td>";
									$M = -1;
									$filled = true;
								}
							}
						}
					}
					
					// If the cell was not filled, fill it with an empty cell.
					if(!$filled)
					{
						$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
					}
					$filled = false;
               	                     
					//---------------TUESDAY---------------
					for($j = 0; $j < $this->nclasses; $j++)
					{
						if($Tu == -1)
						{
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getTu())
							{
								if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
								{
									if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
									{
										$table .= "\n\t\t<td class=\"top\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$Tu = $j;
										$filled = true;
									} else {
										$table .= "\n\t\t<td class=\"single\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$filled = true;
									}
								}
							}
						} else {
							if($j == $Tu)
							{
								if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
								{
									$table .= "\n\t\t<td class=\"mid\">&nbsp;</td>";
									$filled = true;
								} else {
									$table .= "\n\t\t<td class=\"end\">&nbsp;</td>";
									$Tu = -1;
									$filled = true;
								}
							}
						}
					}
					
					// If the cell was not filled, fill it with an empty cell.
					if(!$filled)
					{
						$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
					}
					$filled = false;
	
					//---------------WEDNESDAY---------------
					for($j = 0; $j < $this->nclasses; $j++)
					{
						if($W == -1)
						{
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getW())
							{
								if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
								{
									if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
									{
										$table .= "\n\t\t<td class=\"top\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$W = $j;
										$filled = true;
									} else {
										$table .= "\n\t\t<td class=\"single\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$filled = true;
									}
								}
							}
						} else {
							if($j == $W)
							{
								if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
								{
									$table .= "\n\t\t<td class=\"mid\">&nbsp;</td>";
									$filled = true;
								} else {
									$table .= "\n\t\t<td class=\"end\">&nbsp;</td>";
									$W = -1;
									$filled = true;
								}
							}
						}
					}
					
					// If the cell was not filled, fill it with an empty cell.
					if(!$filled)
					{
						$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
					}
					$filled = false;

					//---------------THURSDAY---------------
					for($j = 0; $j < $this->nclasses; $j++)
					{
						if($Th == -1)
						{
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getTh())
							{
								if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
								{
									if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
									{
										$table .= "\n\t\t<td class=\"top\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$Th = $j;
										$filled = true;
									} else {
										$table .= "\n\t\t<td class=\"single\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$filled = true;
									}
								}
							}
						} else {
							if($j == $Th)
							{
								if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
								{
									$table .= "\n\t\t<td class=\"mid\">&nbsp;</td>";
									$filled = true;
								} else {
									$table .= "\n\t\t<td class=\"end\">&nbsp;</td>";
									$Th = -1;
									$filled = true;
								}
							}
						}
					}
					
					// If the cell was not filled, fill it with an empty cell.
					if(!$filled)
					{
						$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
					}
					$filled = false;

					//---------------FRIDAY---------------
					for($j = 0; $j < $this->nclasses; $j++)
					{
						if($F == -1)
						{
							if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getF())
							{
								if(($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() >= $time[$r]) && ($this->classStorage[$j]->getSection($this->storage[$i][$j])->getStartTime() < $time[$r+1]))
								{
									if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
									{
										$table .= "\n\t\t<td class=\"top\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$F = $j;
										$filled = true;
									} else {
										$table .= "\n\t\t<td class=\"single\">" . $this->classStorage[$j]->getName() . " " . $this->classStorage[$j]->getSection($this->storage[$i][$j])->getLetter() . "</td>";
										$filled = true;
									}
								}
							}
						} else {
							if($j == $F)
							{
								if($this->classStorage[$j]->getSection($this->storage[$i][$j])->getEndTime() > $time[$r+1])
								{
									$table .= "\n\t\t<td class=\"mid\">&nbsp;</td>";
									$filled = true;
								} else {
									$table .= "\n\t\t<td class=\"end\">&nbsp;</td>";
									$F = -1;
									$filled = true;
								}
							}
						}
					}
					
					// If the cell was not filled, fill it with an empty cell.
					if(!$filled)
					{
						$table .= "\n\t\t<td class=\"none\">&nbsp;</td>";
					}
					$filled = false;

					// End of row
					$table .= "\n\t</tr>";
				}

				// End of table
				$table .= "</table></div>";
			}

			echo $header . $table . $footer;   
		} else {
			echo '<html><body><p>There are no possible schedules. Please try again.</p></body></html>';
		}
	}

	//--------------------------------------------------
	// Changes the title of the page.
	//--------------------------------------------------
	function changeTitle($t)
	{
		$this->title = $t;
	}

	//--------------------------------------------------
	// Make the time "pretty."
	//--------------------------------------------------
	function prettyTime($t){
		if($t > 1259)
		{
			$t = ($t-1200);
			return substr($t, 0, strlen($t)-2) . ":" . substr($t, strlen($t)-2, strlen($t)) . " PM";
		} else {
			return substr($t, 0, strlen($t)-2) . ":" . substr($t, strlen($t)-2, strlen($t)) . " AM";
		}
	}

}

?>