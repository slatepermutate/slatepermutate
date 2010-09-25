<?php

/* Class for general page generation */
class page {

  private $base_title = 'SlatePermutate';
  private $doctype = 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';
  private $htmlargs = 'xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"';
  private $bodyargs = '';
  public $lastJobTable = '';
  private $pageGenTime = 0;
  private $indexpath = 'http://protofusion.org/SlatePermutate/'; // full url to index for php header redirection

  // Scripts and styles
  private $headCode = array();

  private $trackingcode = '<script type="text/javascript">
				var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
				document.write(unescape("%3Cscript src=\'" + gaJsHost + "google-analytics.com/ga.js\' type=\'text/javascript\'%3E%3C/script%3E"));
        		   </script>

			  <script type="text/javascript">
				var nathangPageTracker = _gat._getTracker("UA-17441156-1");
				nathangPageTracker._trackPageview();
				
				var ethanzPageTracker = _gat._getTracker("UA-2800455-1");
				ethanzPageTracker._trackPageview();
			</script>'; // Google analytics ga.js tracking code

  private $pagetitle = ''; // Title of page
  private $scripts = array(); // Scripts to include on page

  public function __construct($ntitle, $nscripts = array() ){
    // Scripts and styles available to include
    $this->headCode['jQuery'] = '<script src="http://www.google.com/jsapi"></script><script type="text/javascript" charset="utf-8"> google.load("jquery", "1.3.2"); google.load("jqueryui", "1.7.2");</script>';
    $this->headCode['jValidate'] = '<script type="text/javascript" src="http://ajax.microsoft.com/ajax/jquery.validate/1.7/jquery.validate.pack.js"></script>';
    $this->headCode['schedInput'] = '<script type="text/javascript" src="scripts/scheduleInput.js"></script>';
    $this->headCode['outputPrintStyle'] = '<link rel="stylesheet" href="styles/print.css" type="text/css" media="screen" charset="utf-8">';
    $this->headCode['outputStyle'] = '<link rel="stylesheet" href="styles/output.css" type="text/css" media="screen" charset="utf-8">'; 
    $this->headCode['gliderHeadcode'] = '<link rel="stylesheet" href="styles/glider.css" type="text/css" media="screen" charset="utf-8"><script src="scripts/prototype.js" type="text/javascript" charset="utf-8"></script><script src="scripts/effects.js" type="text/javascript" charset="utf-8"></script><script src="scripts/glider.js" type="text/javascript" charset="utf-8"></script>'; 

   $this->pagetitle = $ntitle;
    $this->scripts = $nscripts;
    if($ntitle != "NOHEAD")
      $this->head();

 }

  private function top(){
    echo '<div id="header">
          <h1><em>SlatePermutate</em> - '.$this->pagetitle.'</h1>
          </div>
          <div id="content">';
  }

// Public functions/vars

  private function head(){
    session_start();
    $this->pageGenTime = round(microtime(), 3);

    echo '<!DOCTYPE ' . $this->doctype . '>
	  <html ' . $this->htmlargs . '>
	  <head>
	    <title>' . $this->pagetitle . ' :: ' . $this->base_title . '</title>
           <link rel="stylesheet" href="styles/general.css" type="text/css" media="screen" charset="utf-8">';

    // Write out all passed scripts
    foreach ($this->scripts as $i){
    	echo $this->headCode["$i"];
    }

    echo '</head>
	  <body '.$this->bodyargs.' >';
    echo $this->top(); // Write out top
  }

  public function foot(){
    echo '</div>';
    $this->pageGenTime = round(microtime(), 3);
    echo '<div id="footer"><h5>&copy; '. date('Y').' <a href="http://protofusion.org/~nathang/">Nathan Gelderloos</a><br /> with special thanks to <a href="http://ethanzonca.com">Ethan Zonca</a></h5></div>';
    echo $this->trackingcode;
    echo '</body></html>';
  }

  public function secondsToCompound($seconds) {
      $ret = "";
      $hours = intval(intval($seconds) / 3600);
      $ret .= "$hours:";
      $minutes = bcmod((intval($seconds) / 60),60);
      $ret .= "$minutes:";
      $seconds = bcmod(intval($seconds),60);
      $ret .= "$seconds";
      return $ret;
  }

  public function showSavedScheds($session) {
       echo '<p>';
	if(isset($session['saved']) && count($session['saved']) > 0){
		echo '<div id="savedBox" ><h3>Saved Schedules:</h3>';
		foreach($session['saved'] as $key => $schedule){
			$sch = unserialize($schedule);
			echo "<a href=\"process.php?savedkey=$key\">#" . ($key + 1) . " - " . $sch->getName() . "</a> <em><a href=\"process.php?delsaved=$key\"><img src=\"images/close.png\" style=\"border:0;\" /></a></em><br />";
		}
		echo '</div>';
	}
       echo '</p>';
}

}
