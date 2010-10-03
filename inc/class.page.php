<?php

/* Class for general page generation */
class page
{
  private $base_title = 'SlatePermutate';
  private $doctype = 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';
  private $htmlargs = 'xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"';
  private $bodyargs = '';
  public $lastJobTable = '';
  private $pageGenTime = 0;
  private $indexpath = 'http://protofusion.org/SlatePermutate/'; // full url to index for php header redirection
  /* whether or not to output valid XHTML */
  private $xhtml = FALSE;

  // Scripts and styles
  private $headCode = array();

  /* the inclusion of ga.js is augmented in __construct(). */
  private $trackingcode = '
			  <script type="text/javascript">
				var nathangPageTracker = _gat._getTracker("UA-17441156-1");
				nathangPageTracker._trackPageview();
				
				var ethanzPageTracker = _gat._getTracker("UA-2800455-1");
				ethanzPageTracker._trackPageview();
			</script>'; // Google analytics ga.js tracking code

  private $pagetitle = ''; // Title of page
  private $scripts = array(); // Scripts to include on page

  public function __construct($ntitle, $nscripts = array(), $immediate = TRUE)
  {
    // Scripts and styles available to include
    $this->headCode['jQuery'] = '<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.3/jquery.min.js" type="text/javascript" />';
    $this->headCode['jQueryUI'] = '<script src="http://ajax.googleapis.com/ajax/libs/jqueryui/1.7/jquery-ui.min.js" type="text/javascript" />';
    $this->headCode['jValidate'] = '<script type="text/javascript" src="http://ajax.microsoft.com/ajax/jquery.validate/1.7/jquery.validate.pack.js"></script>';
    $this->headCode['schedInput'] = '<script type="text/javascript" src="scripts/scheduleInput.js"></script>';
    $this->headCode['outputPrintStyle'] = '<link rel="stylesheet" href="styles/print.css" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['outputStyle'] = '<link rel="stylesheet" href="styles/output.css" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['gliderHeadcode'] = '<link rel="stylesheet" href="styles/glider.css" type="text/css" media="screen" charset="utf-8" /><script src="scripts/prototype.js" type="text/javascript" charset="utf-8"></script><script src="scripts/effects.js" type="text/javascript" charset="utf-8"></script><script src="scripts/glider.js" type="text/javascript" charset="utf-8"></script>'; 

   $this->pagetitle = $ntitle;
   $this->scripts = $nscripts;

   /* compliant browsers which care, such as gecko, explicitly request xhtml: */
   if(!empty($_SERVER['HTTP_ACCEPT'])
      && strpos($_SERVER['HTTP_ACCEPT'], 'application/xhtml+xml') !== FALSE
      || !strlen($_SERVER['HTTP_ACCEPT']) /* then the browser doesn't care :-) */)
     {
       $this->xhtml = TRUE;
       header('Content-type: application/xhtml+xml');
     }

   $ga_www = 'http://www.';
   if ($_SERVER['SERVER_PORT'] != 80)
     $ga_www = 'https://ssl.';
   $this->trackingcode = '<script type="text/javascript" src="' . $ga_www . 'google-analytics.com/ga.js" />' . "\n"
     . $this->trackingcode;

    session_start();
    if($immediate
       && $ntitle != "NOHEAD")
      $this->head();
 }

  /**
   * \brief
   *   Adds some headcode to this page.
   *
   * \param $key
   *   The key to register this headcode under.
   * \param $code
   *   The actuall code, such as a <script/>.
   * \param $enable
   *   Whether or not to enable this code while adding it.
   */
  public function headcode_add($key, $code, $enable = FALSE)
  {
    $this->headCode[$key] = $code;
    if ($enable)
      $this->scripts[] = $key;
  }

  private function top(){
    echo '<div id="header">
	    <h2 id="title"><a href="index.php"><img src="images/slatepermutate.png" alt="SlatePermutate" class="noborder" /></a><br /><span style="margin-left: 1em;">'.$this->pagetitle.'</span></h2>
	    
	    <span id="menu">
	      <!-- <a href="index.php">Home</a> :: <a href="input.php">Scheduler</a> :: <a href="about.php">About</a> -->
	    </span>
	  </div>
          <div id="content">';
  }

// Public functions/vars

  public function head(){
    $this->pageGenTime = round(microtime(), 3);

    if ($this->xhtml)
       echo '<?xml version="1.0" encoding="utf-8" ?>' . "\n";

    echo '<!DOCTYPE ' . $this->doctype . '>
	  <html ' . $this->htmlargs . '>
	  <head>
	    <title>' . $this->pagetitle . ' :: ' . $this->base_title . '</title>
           <link rel="stylesheet" href="styles/general.css" type="text/css" media="screen" charset="utf-8" />';

    // Write out all passed scripts
    foreach ($this->scripts as $i){
    	echo $this->headCode["$i"];
    }

    echo '</head>
	  <body '.$this->bodyargs.' ><div id="page">';
    echo $this->top(); // Write out top
  }

  public function foot(){
    echo '</div> <!-- id="content" -->';
    $this->pageGenTime = round(microtime(), 3);
    echo '  <div id="footer">
    <h5>&copy; '. date('Y').' <a href="http://protofusion.org/~nathang/">Nathan Gelderloos</a><br />
      <a href="http://ethanzonca.com">Ethan Zonca</a>
    </h5>
  </div> <!-- id="footer" -->
</div>';
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

  public function showSavedScheds($session)
  {
    echo '<p>';
    if (isset($session['saved']) && count($session['saved']) > 0)
      {
	echo '<div id="savedBox" ><h3>Saved Schedules:</h3>';
	foreach($session['saved'] as $key => $name)
	  {
	    echo '<a href="process.php?s=' . $key . '" title="View schedule #' . $key . '">#' . $key . "</a>:\n "
	      . htmlentities($name)
	      . ' <a href="input.php?s=' . $key . '">edit</a>'
	      . ' <a href="process.php?del=' . $key . '">delete</a>'
	      . "<br /><br />\n";
	  }
	echo '</div>';
      }
    echo '</p>';
  }

  /**
   * \brief
   *   Display a 404 page and halt the PHP interpreter.
   *
   * This function does not return. It handles the creation of a Page
   * class with 404-ish stuff and then calls exit() after flushing the
   * page out to the user.
   *
   * \param $message
   *   A message consisting of valid XHTML to display to the user in
   *   the 404 page.
   */
  public static function show_404($message = 'I couldn\'t find what you were looking for :-/.')
  {
    $page_404 = new Page('404: Content Not Found');

    echo "<h2>404: Content Not Found</h2>\n"
      . "<p>\n"
      . '  ' . $message . "\n"
      . "</p>\n";

    $page_404->foot();

    exit();
  }
}
