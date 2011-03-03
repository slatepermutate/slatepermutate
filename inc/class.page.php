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

/*
 * The @PACKAGE_*@-style defines.
 */
define('SP_PACKAGE_NAME', 'slate_permutate');
define('SP_PACKAGE_VERSION', '0.1_pre');
define('SP_PACKAGE_STRING', SP_PACKAGE_NAME . '-' . SP_PACKAGE_VERSION);

/*
 * Set up include() path for user-supplied libs (in case if his system
 * doesn't have libs, such as phpcaptcha
 * (securimage/securimage.php)). Users would store such libs in /libs.
 *
 * Coding note: dirname(dirname('a/b/c')) returns 'a'. This is a
 * similar effect to dirname('a/b/c') . DIRECTORY_SEPARATOR . '..'.
 */
set_include_path(get_include_path() . PATH_SEPARATOR . dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'libs');

/**
 * Not sure if there's a better place for this... it'd be a pita to
 * make a new include file like doconfig.inc but maybe that'll make
 * sense soon.
 */
/* defaults */
$clean_urls = FALSE;
$ga_trackers = array();
$feedback_emails = array('ez@ethanzonca.com, ngelderloos7@gmail.com, ohnobinki@ohnopublishing.net');
$use_captcha = FALSE;
$admin_enable_purge = FALSE;
$qtips_always = FALSE;

$config_inc = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config.inc';
if (file_exists($config_inc)) {
  require_once($config_inc);
}


//**************************************************
// class.page.php   Author: Ethan Zonca
//
// Provides an interface for generating a styled
// XHTML page, supporting modular script inclusion
// and other various features
//**************************************************
class page
{

  /* Site-wide configuration options */
  private $base_title = 'SlatePermutate - Find the schedule that works for you!';
  private $doctype = 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';
  private $htmlargs = 'xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"';

  private $pageGenTime = 0;

  private $xhtml = FALSE;

  /* Scripts and styles */
  private $headCode = array();

  private $trackingcode = ''; // Tracking code
  private $pagetitle = ''; // Title of page
  private $scripts = array(); // Scripts to include on page

  /* the current school. See get_school(). */
  private $school;


  /**
   * \param $ntitle
   *   Must be a valid HTML string (i.e., escaped with htmlentities()).
   * \param $nscripts
   *   An array of strings identifying the scripts to include for this page.
   */
  public function __construct($ntitle, $nscripts = array(), $immediate = TRUE)
  {
    /* Begin tracking generation time */
    $this->pageGenTime = round(microtime(),4);

    global $ga_trackers;

    require_once('school.inc');

    /* Scripts and styles available for inclusion */
    $this->headCode['jQuery'] = '<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js" type="text/javascript"></script>';
    $this->headCode['jQueryUI'] = '<script src="http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js" type="text/javascript"></script><link rel="stylesheet" href="styles/jqueryui.css" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['jValidate'] = '<script type="text/javascript" src="http://ajax.microsoft.com/ajax/jquery.validate/1.7/jquery.validate.pack.js"></script>';
    $this->headCode['jAddress'] = '<script type="text/javascript" src="http://ohnopub.net/js/jquery.address-1.3.2.min.js"></script>';
    $this->headCode['qTip'] = '<script type="text/javascript" src="http://ohnopub.net/js/jquery.qtip-1.0.min.js"></script>';
    $this->headCode['schedInput'] = '<script type="text/javascript" src="scripts/scheduleInput.js"></script>';
    $this->headCode['outputPrintStyle'] = '<link rel="stylesheet" href="styles/print.css" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['outputStyle'] = '<link rel="stylesheet" href="styles/output.css" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['gliderHeadcode'] = '<link rel="stylesheet" href="styles/glider.css" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['uiTabsKeyboard'] = '<script type="text/javascript" src="scripts/uiTabsKeyboard.js"></script>';
    $this->headCode['displayTables'] = '<script type="text/javascript" src="scripts/displayTables.js"></script>';

    $this->pagetitle = $ntitle;
    $this->scripts = $nscripts;

   /* Compliant browsers which care, such as gecko, explicitly request xhtml: */
   if(empty($_SERVER['HTTP_ACCEPT'])  /* then the browser doesn't care :-) */
      || strpos($_SERVER['HTTP_ACCEPT'], 'application/xhtml+xml') !== FALSE)
     {
       $this->xhtml = TRUE;
       header('Content-Type: application/xhtml+xml; charset=utf-8');
     }
   else
     header('Content-Type: text/html; charset=utf-8');

   if (count($ga_trackers))
     {
       $ga_www = 'http://www.';
       if ($_SERVER['SERVER_PORT'] != 80)
	 $ga_www = 'https://ssl.';

       $this->trackingcode = '<script type="text/javascript" src="' . $ga_www . 'google-analytics.com/ga.js" />' . "\n"
	 . $this->trackingcode
	 . '  <script type="text/javascript">' . "\n"
	 . '  ' . ($this->xhtml ? '<![CDATA[' : '') . "\n"
	 . "     var mytrackers = new Array();";

       $i = 0;
       foreach ($ga_trackers as $ga_tracker)
	 {
	   $this->trackingcode .= "\n"
	     . '      mytrackers[' . $i . '] = _gat._getTracker(\'' . $ga_tracker . "');\n"
	     . '      mytrackers[' . $i . "]._trackPageview();\n";
	 }

       $this->trackingcode .= '  ' . ($this->xhtml ? ']]>'       : '') . "\n"
	 . "  </script>\n";
     }

    self::session_start();
    /* everything that needs sessions started to work: */

    $this->school = school_load_guess();
    $this->semester = school_semester_guess($this->school);

    if($immediate
       && $ntitle != "NOHEAD")
      $this->head();
 }

  /**
   * \brief
   *   Instantiate a new page for the caller.
   *
   * The caller must explicitly call the page::head() function upon
   * the value that is returned. No implicit actions are supported
   * anymore.
   *
   * \param $title
   *   The title of the page. Must be completely UTF-8 (will be
   *   escaped for you with htmlentitites()).
   * \param $scripts
   *   A list of scripts which the page desires to be included in the
   *   <head /> of the page. Should this param just be moved to the
   *   page::head() function?
   */
  public static function page_create($title, array $scripts = array())
  {
    return new page(htmlentities($title), $scripts, FALSE);
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
    if ($enable) {
      $this->scripts[] = $key;
    }
  }

  /**
   * \brief
   *   Output the HTML header for a page, including <!DOCTYPE>, <head />, and opening structure
   */
  public function head()
  {

    if ($this->xhtml) {
      echo '<?xml version="1.0" encoding="utf-8"?>' . PHP_EOL;
    }

    echo '<!DOCTYPE ' . $this->doctype . '>'. PHP_EOL .
	  '<html ' . $this->htmlargs . '>'. PHP_EOL .
	  '  <head>'. PHP_EOL .
	  '    <title>' . $this->pagetitle . ' - ' . $this->base_title . '</title>'. PHP_EOL .
          '    <link rel="stylesheet" href="styles/general.css" type="text/css" media="screen" charset="utf-8" />'.  PHP_EOL .
	  '    <link rel="stylesheet" type="text/css" media="print" href="styles/print.css" />'. PHP_EOL .
          '    <link rel="shortcut icon" href="images/favicon.png" />'. PHP_EOL;

    // Write out all passed scripts
    foreach ($this->scripts as $i)
      echo '    ' . $this->headCode["$i"] . "\n";

    echo '  </head>' . PHP_EOL .
	 '  <body>'. PHP_EOL .
         '    <div id="page">'. PHP_EOL .
         '      <div id="header">'. PHP_EOL .
	 '        <div id="title">'. PHP_EOL .
         '          <h1><a href="index.php"><img src="images/slatepermutate-alpha.png" alt="SlatePermutate" class="noborder" /></a><br /></h1>'. PHP_EOL .
         '          <p>'. PHP_EOL .
         '            <span id="subtitle">'.$this->pagetitle.'</span>'. PHP_EOL .
  	 '            <span id="menu">' . PHP_EOL
      . '              Profile: '.$this->school['name'].' <a href="input.php?selectschool=1">(change)</a>' . PHP_EOL;
    if ($this->semester !== NULL)
      echo  '             Semester: ' . $this->semester['name'] . '<a href="input.php?selectsemester=1">(change)</a>' . PHP_EOL;
    echo '            </span>'. PHP_EOL .
         '          </p>'. PHP_EOL .
         '        </div>'. PHP_EOL .
	 '      </div>'. PHP_EOL .
         '      <div id="content">'. PHP_EOL;
  }

  /**
   * \brief
   *   Write out the foot of the page and closing divs
   */
  public function foot(){
    echo '      </div> <!-- id="content" -->'. PHP_EOL;
    echo '      <div id="footer">'. PHP_EOL .
  	 '        <div id="leftfoot" style="float:left; margin-top: 1em;">'. PHP_EOL .
	 '          <a href="feedback.php">Submit Feedback</a>'. PHP_EOL .
         '        </div>'. PHP_EOL .
         '        <div id="rightfoot">'. PHP_EOL .
         '          <h5>&copy; '. date('Y').' <a href="http://protofusion.org/~nathang/">Nathan Gelderloos</a><br /><a href="http://ethanzonca.com">Ethan Zonca</a><br /><a href="http://ohnopub.net">Nathan Phillip Brink</a></h5>'. PHP_EOL .
	 '        </div>'. PHP_EOL .
         '      </div> <!-- id="footer" -->'. PHP_EOL .
         '    </div> <!-- id="page" -->'. PHP_EOL;
    echo $this->trackingcode;
    echo '  </body>'. PHP_EOL .
         '</html>' . PHP_EOL;
    $this->pageGenTime = round(microtime() - $this->pageGenTime,4);
    echo '<!-- Page generated in ' . $this->pageGenTime . ' seconds -->';
  }

  /**
   * \brief
   *   Shows a box with recently processed schedules
   */
  public function showSavedScheds($session) {
    global $clean_urls;

    if (isset($session['saved']) && count($session['saved']) > 0) {
      echo '<div id="savedBox">' . PHP_EOL;

      $process_php_s = 'process.php?s=';
      if ($clean_urls) {
        $process_php_s = '';
      }

      echo '<h3>Saved Schedules:</h3>';

      $hidden = 'hidden';
      $numsaved = count($session['saved']);
      $count = $numsaved;
      $output = '';

      foreach($session['saved'] as $key => $name)
	{
	  if($count <= 4)
	    $hidden = '';

	  $output =  '<p class="' . $hidden . '">'  . PHP_EOL
	    . '  <a href="' . $process_php_s . $key . '" title="View schedule #' . $key . '">#' . $key . "</a>:" 
	    . htmlentities($name)
	    . ' <a href="input.php?s=' . $key . '">edit</a>'
	    . ' <a href="process.php?del=' . $key . '">delete</a>'
	    . ' <br /><br />' . PHP_EOL
	    . '</p>' . PHP_EOL . $output;

          $count --;
	}
      echo $output;
      if ($numsaved > 4)
	echo '<div id="showLess"><a href="#">Less...</a></div>' . PHP_EOL
	  . '<div id="showMore"><a href="#">More...</a></div>' . PHP_EOL;
      echo '</div>' . PHP_EOL;
    }
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
    echo school_list_html($this->school['id'], $linkto);
    echo "</p>\n";
  }

  /**
   * \brief
   *   Display a list of semesters the user might be interested in.
   * \param $linkto
   *   The link to which a &semester= or ?semester= query string
   *   should be appended.
   */
  public function showSemesters($linkto = 'input.php')
  {
    if (strpos($linkto, '?'))
      $linkto .= '&';
    else
      $linkto .= '?';
    /*
     * We can pre-htmlentities() $linkto because we're only appending
     * a safe string.
     */
    $linkto = htmlentities($linkto . 'semester=');

    $time = time();

    echo "    <p>\n";
    echo "      <ul>\n";
    foreach (school_semesters($this->school) as $semester)
      {
	$text_extra = array();
	$class_extra = '';
	if ($semester['id'] == $this->semester['id'])
	  {
	    $class_extra = ' highlight';
	    $text_extra[] = 'selected';
	  }

	if ($semester['time_start'] < $time && $semester['time_end'] > $time)
	  $text_extra[] = 'current';

	$text_extra = implode($text_extra, ', ');
	if (strlen($text_extra))
	  $text_extra = ' (' . $text_extra . ')';

	echo '        <li class="semester' . $class_extra . '"><a href="' . $linkto . $semester['id'] . '">' . htmlentities($semester['name']) . '</a>' . $text_extra . "</li>\n";
      }
    echo "      </ul>\n";
    echo "    </p>\n";
  }

  /**
   * \brief
   *   Display school-specific instructions for using slate_permutate.
   */
  public function showSchoolInstructions()
  {
    echo '<div id="schoolInstructionsBox">' . PHP_EOL
       . school_instructions_html($this->school) . PHP_EOL
       . '</div> <!-- id="schoolInstructionsBox" -->' . PHP_EOL;
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
    if (!$necessary && $this->school['id'] == 'default') {
      return;
    }
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
  public static function show_404($message = 'The page you were looking for cannot be found!.')
  {
    $page_404 = page::page_create('404: Content Not Found');
    $page_404->head();

    echo "<h2>404: Content Not Found</h2>\n"
      . "<p>\n"
      . '  ' . $message . "\n"
      . "</p>\n";

    $page_404->foot();

    exit();
  }

  /**
   * \brief
   *   Start the PHP session by calling session_start().
   *
   * Used to make sure that different areas of our code don't call
   * session_start() multiple times and to make it easier to ensure
   * that session_start() is called at least before it's needed.
   */
  public static function session_start()
  {
    static $session_started = FALSE;

    if (!$session_started)
      {
	session_name('slate_permutate');
	session_start();
	$session_started = TRUE;
      }
  }

  /**
   * \brief
   *   Perform a redirect.
   *
   * By consolidating all redirects here, we're hopefully able to do
   * it in a somewhat compliant and portablish way ;-).
   *
   * This function does not return. It calls exit().
   *
   * \param $dest
   *   A URL relative to the slate_permutate root. For example,
   *   'input.php' or '44' (for clean urls, for example).
   * \param $http_code
   *   The redirection code to use, if any. For example, this can be
   *   used to implement ``permanent'' redirects if necessary.
   */
  public static function redirect($dest, $http_code = NULL)
  {
    if ($http_code)
      header('HTTP/1.1 ' . $http_code);

    $uri = '';

    $host = '';
    if (isset($_SERVER['SERVER_NAME']))
      $host = $_SERVER['SERVER_NAME'];
    if (isset($_SERvER['HTTP_HOST']))
      $host = $_SERVER['HTTP_HOST'];

    if (strlen($host))
      {
	$proto = 'http';
	$port = NULL;
	if (isset($_SERVER['SERVER_PORT']) && $_SERVER['SERVER_PORT'] != 80)
	  {
	    if ($_SERVER['SERVER_PORT'] == 443 || !empty($_SERVER['HTTPS']))
	      $proto .= 's';
	    if ($_SERVER['SERVER_PORT'] != 433)
	      $port = $_SERVER['SERVER_PORT'];
	  }

	$uri = $proto . '://' . $host;
	if ($port !== NULL)
	  $uri .= ':' . $port;
	$uri .= dirname($_SERVER['REQUEST_URI']) . '/';
      }

    header('Location: ' . $uri . $dest);

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

  /**
   * \brief
   *   Format a chunk of javascript suitable for adding to headcode.
   *
   * Takes into account whether or not the code should be wrapped in
   * CDATA or not.
   *
   * \param $js
   *   The javascript to wrap up.
   * \param $type
   *   The type="" attribute of the <script/> element
   */
  public function script_wrap($js, $type = 'text/javascript')
  {
    return '<script type="' . $type . '">' . PHP_EOL
         . ($this->xhtml ? '<![CDATA[' : '') . PHP_EOL
         . $js . PHP_EOL
	 . ($this->xhtml ? ']]>' : '') . PHP_EOL
         . '// </script>';
  }

  /**
   * \brief
   *   Add a trailing slash to a path if one does not already exist
   */
  private function add_trailing_slash($path){
    if($path[strlen($path)-1] != '/') {
      return $path . "/";
    }
    else {
      return $path;
    }
  }

  /**
   * \brief
   *   Generate a URL to a given schedule.
   *
   * \return
   *   The URL used to access the schedule. You must call
   *   htmlentities() on this string if it is to be inserted into an
   *   XHTML document.
   */
  public function gen_share_url($id)
  {
    global $clean_urls, $short_url_base;

    if ($clean_urls && isset($short_url_base)) {
      return $this->add_trailing_slash($short_url_base) . $id;
    }
    elseif ($clean_urls) {
      return 'http://' . $_SERVER['HTTP_HOST'] . dirname($_SERVER['REQUEST_URI']) . '/' . $id;
    }
    else {
      return 'http://' . $_SERVER['HTTP_HOST']  . dirname($_SERVER['REQUEST_URI']) . '/process.php?s=' . $id;
    }
  }

  /**
   * \brief
   *   Generate special code to close a self-closing XHTML/HTML
   *   element.
   *
   * \return
   *   A string containing the correct self-closing chars. For
   *   example, this would be ' /' for XHTML.
   */
  public function element_self_close()
  {
    if ($this->xhtml)
      return ' /';
    return '';
  }
}
