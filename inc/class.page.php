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

  /* the current school. See get_school(). */
  private $school;

  public function __construct($ntitle, $nscripts = array(), $immediate = TRUE)
  {
    require_once('school.inc');

    // Scripts and styles available to include
    $this->headCode['jQuery'] = '<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.3/jquery.min.js" type="text/javascript" />';
    $this->headCode['jQueryUI'] = '<script src="http://ajax.googleapis.com/ajax/libs/jqueryui/1.7/jquery-ui.min.js" type="text/javascript" /><link rel="stylesheet" href="styles/jqueryui.css" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['jValidate'] = '<script type="text/javascript" src="http://ajax.microsoft.com/ajax/jquery.validate/1.7/jquery.validate.pack.js"></script>';
    $this->headCode['schedInput'] = '<script type="text/javascript" src="scripts/scheduleInput.js"></script>';
    $this->headCode['outputPrintStyle'] = '<link rel="stylesheet" href="styles/print.css" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['outputStyle'] = '<link rel="stylesheet" href="styles/output.css" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['gliderHeadcode'] = '<link rel="stylesheet" href="styles/glider.css" type="text/css" media="screen" charset="utf-8" />'; 
   $this->headCode['uiTabsKeyboard'] = '<script type="text/javascript" src="scripts/uiTabsKeyboard.js"></script>'; 
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

    /* everything that needs sessions started to work: */

    $this->school = school_load_guess();
 }

  /**
   * \brief
   *   Adds some headcode to this page.
   *
   * \param $key
   *   The key to register this headcode under.
   * \param $code
   *   The actual code, such as a <script/>.
   * \param $enable
   *   Whether or not to enable this code while adding it.
   */
  public function headcode_add($key, $code, $enable = FALSE)
  {
    $this->headCode[$key] = $code;
    if ($enable)
      $this->scripts[] = $key;
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

  private function top(){
    echo '<div id="header">

	    <div id="title">
              <h1><a href="index.php"><img src="images/slatepermutate.png" alt="SlatePermutate" class="noborder" /></a><br /></h1>
              <p><span id="subtitle">'.$this->pagetitle.'</span>
  	      <span id="menu">Profile: '.$this->school['name'].' <a href="input.php?selectschool=1">(change)</a></span>

              </p>



            </div>

	  </div>
          <div id="content">';
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
   *   Display a list of schools the user might be from.
   * \param $linkto
   *   The to which a &school= or ?school= query string should be
   *   appended.
   */
  public function showSchools($linkto)
  {
    echo "<p>\n";
    echo "  <div id=\"schoolBox\">\n";
    echo school_list_html($this->school['id'], $linkto);
    echo "  </div> <!-- id=\"schoolBox\" -->\n";
    echo "</p>\n";
  }

  /**
   * \brief
   *   Display school-specific instructions for using slate_permutate.
   */
  public function showSchoolInstructions()
  {
    echo "<div id=\"schoolInstructionsBox\">\n";
    echo school_instructions_html($this->school);
    echo "</div> <!-- id=\"schoolInstructionsBox\" -->\n";
  }

  /**
   * \brief
   *   Print out a vocative form of a student's identity. For example,
   *   Dearborn Christin Schoolers are called ``Knights'' as are
   *   Calvin College students.
   *
   * The third argument is used to determine whether or not this
   * address _needs_ to be printed out. For example, in some sentences
   * when addressing generic students, it makes no sense to say the
   * standard ``Welcome, student'' or ``Dear generic person, how do
   * you do today?''. If the third argument is false, we'll refrain
   * from outputting anything at all.
   *
   * \param $prefix
   *   If the address is to be printed, output this beforehand. Useful
   *   if this prefix shouldn't be printed if the address itself isn't
   *   to be printed. See $necessary.
   * \param $postfix
   *   Text to print after the address if it's printed.
   * \param $necessary
   *   Whether or not we might ignore the request that an address be
   *   printed in certain cases. We default to always printing the
   *   address.
   */
  public function addressStudent($prefix = '', $postfix = '', $necessary = TRUE)
  {
    if (!$necessary && $this->school['id'] == 'default')
      return;

    echo $prefix . $this->school['student_address'] . $postfix;
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

  /**
   * \brief
   *   Get the current school profile handle.
   */
  public function get_school()
  {
    return $this->school;
  }
}
