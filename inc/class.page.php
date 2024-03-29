<?php /* -*- mode: php; -*- */
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
$base_uri = '';
$clean_urls = FALSE;
$ga_trackers = array();
$ga_conversions = array();
$feedback_emails = array('ez@ethanzonca.com, ngelderloos7@gmail.com, ohnobinki@ohnopublishing.net');
$use_captcha = FALSE;
$admin_enable_purge = FALSE;
$qtips_always = FALSE;
$input_warning_banner = FALSE;
$feedback_disk_log = FALSE;
$ratelimit_maxschedules = 16;
$ratelimit_destination = 'http://comeget.ohnopub.net/' . (rand(0, 1) ? 'pul.jpg' : 'jdl.gif');

$config_inc = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config.inc';
if (file_exists($config_inc)) {
  require_once($config_inc);
}


/**
 * \brief
 *   Produces XHTML output for the user and helps with other browser
 *   interaction.
 *
 * Supports styled XHTML pages, modular script inclusion, and other
 * various features.
 */
class page
{

  /* Site-wide configuration options */
  private $base_title = array('SlatePermutate', 'Find the schedule that works for you!');
  private $doctype = 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';
  private $htmlargs = 'xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"';

  private $pageGenTime = 0;

  private $xhtml = FALSE;

  /* Scripts and styles */
  private $headCode = array();

  private $trackingcode = ''; // Tracking code
  private $ga_conversions_code = ''; // Conversion tracking code
  private $pagetitle = ''; // Title of page
  private $scripts = array(); // Scripts to include on page
  private $meta;

  /* the current school. See get_school(). */
  private $school;

  private $semester;

  /*
   * Whether or not the user should be presented with the option to
   * change the school profile or semester.
   */
  private $school_semester_constant;

  /**
   * \brief
   *   Construct a new page.
   *
   * Only to be called by page::page_construct(). Use that function
   * instead of the new keyword.
   *
   * \see page::page_construct()
   *
   * \param $ntitle
   *   Must be a valid HTML string (i.e., escaped with htmlentities()).
   * \param $nscripts
   *   An array of strings identifying the scripts to include for this page.
   * \param $options
   *   An array containing any of the following keys:
   *     - 'school': The school to use instead of the autodetected one.
   *     - 'semester': The semester to use instead of the autodetected one.
   */
  protected function __construct($ntitle, $nscripts = array(), $immediate = TRUE, array $options = array())
  {
    /* Begin tracking generation time */
    $this->pageGenTime = round(microtime(TRUE),4);

    global $cache_buster;
    global $ga_trackers;

    require_once('school.inc');

    $desired_lang = empty($_SERVER['HTTP_ACCEPT_LANGUAGE']) ? '' : $_SERVER['HTTP_ACCEPT_LANGUAGE'];
    $desired_lang = preg_split('/[;,]/', $desired_lang . ',en');
    foreach ($desired_lang as $l) {
      $l = substr($l, 0, 2);
      switch ($l) {
      case 'en':
      case 'ja':
      case 'ko':
        break;
      default:
        $l = '';
        break;
      }
      if (!empty($l)) {
        $desired_lang = $l;
        break;
      }
    }

    $this->desired_lang = $desired_lang;
    $this->slate_t = $desired_lang == 'ko' ? '스레이트' : ($desired_lang == 'ja' ? 'スレート' : 'Slate');
    $this->title_font_size_multiplier = $desired_lang == 'ko' ? '1.2' : ($desired_lang == 'ja' ? '1.5' : '1.5');
    $this->title_uses_svg = $this->slate_t == 'Slate';
    $this->permutate_t = $desired_lang == 'ko' ? '퍼뮤테이트' : ($desired_lang == 'ja' ? 'パーミュテート' : 'Permutate');
    $this->base_title[0] = $this->slate_t . $this->permutate_t;

    /* Scripts and styles available for inclusion */

    $cb = '?_=' . htmlentities(rawurlencode(empty($cache_buster) ? '' : $cache_buster), ENT_QUOTES);
    $this->headCode['jQuery'] = '<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.6/jquery.min.js" type="text/javascript"></script>';
    $this->headCode['jQueryUI'] = '<script src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js" type="text/javascript"></script><link rel="stylesheet" href="styles/jqueryui.css' . $cb . '" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['jValidate'] = '<script type="text/javascript" src="https://ajax.aspnetcdn.com/ajax/jquery.validate/1.7/jquery.validate.pack.js"></script>';
    $this->headCode['jAddress'] = '<script type="text/javascript" src="//js.ohnopub.net/js/jquery.address-1.3.2.min.js"></script>';
    $this->headCode['jQuery.cuteTime'] = '<script type="text/javascript" src="//js.ohnopub.net/js/ohnobinki/2011.04.19/jquery.cuteTime.min.js"></script>';
    $this->headCode['qTip'] = '<script type="text/javascript" src="//js.ohnopub.net/js/jquery.qtip-1.0.min.js"></script>';
    $this->headCode['qTip2'] = '<script type="text/javascript" src="//js.ohnopub.net/js/2011.03.21/jquery.qtip.min.js"></script><link rel="stylesheet" href="//js.ohnopub.net/js/2011.03.21/jquery.qtip.min.css" type="text/css" media="screen" />';
    $this->headCode['schedInput'] = '<script type="text/javascript" src="scripts/scheduleInput.js' . $cb . '"></script>';
    $this->headCode['outputPrintStyle'] = '<link rel="stylesheet" href="styles/print.css' . $cb . '" type="text/css" media="screen" charset="utf-8" />';
    $this->headCode['outputStyle'] = '<link rel="stylesheet" href="styles/output.css' . $cb . '" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['gliderHeadcode'] = '<link rel="stylesheet" href="styles/glider.css' . $cb . '" type="text/css" media="screen" charset="utf-8" />'; 
    $this->headCode['uiTabsKeyboard'] = '<script type="text/javascript" src="scripts/uiTabsKeyboard.js' . $cb . '&amp;v=20121128h"></script>';
    $this->headCode['displayTables'] = '<script type="text/javascript" src="scripts/displayTables.js' . $cb . '"></script>';

    $this->pagetitle = $ntitle;
    $this->scripts = $nscripts;
    $this->meta = array(
      'viewport' => 'initial-scale=1',
      'msapplication-starturl' => self::uri_resolve(''),
      'msapplication-task' => 'name=Create Schedule; action-uri=' . self::uri_resolve('input.cgi') . '; icon-uri=' . self::uri_resolve('images/favicon_96.png'),
      'msapplication-tooltip' => 'Find the semester schedule which works for you!',
    );

   /* Compliant browsers which care, such as gecko, explicitly request xhtml: */
   if(empty($_SERVER['HTTP_ACCEPT'])  /* then the browser doesn't care :-) */
      || strpos($_SERVER['HTTP_ACCEPT'], 'application/xhtml+xml') !== FALSE)
       $this->xhtml = TRUE;

   if (count($ga_trackers))
     {
       $ga_www = 'http://www.';
       if ($_SERVER['SERVER_PORT'] != 80)
	 $ga_www = 'https://ssl.';

       $this->trackingcode .=
	  '  <script type="text/javascript">' . "\n"
	 . '  ' . ($this->xhtml ? '<![CDATA[' : '') . "\n"
         // Snippet from https://developers.google.com/analytics/devguides/collection/upgrade/reference/gajs-analyticsjs
	 . "     (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
})(window,document,'script','//www.google-analytics.com/analytics.js','ga');\n";

       foreach ($ga_trackers as $ga_tracker_key => $ga_tracker)
	 {
     $trackerName = "t$ga_tracker_key";
	   $this->trackingcode .= "\n"
	     . '      ga(\'create\', ' . json_encode($ga_tracker) . ", 'auto', " . json_encode($trackerName) . ");\n"
	     . "      ga(" . json_encode("$trackerName.send") . ", 'pageview');\n";
	 }

       $this->trackingcode .= '  ' . ($this->xhtml ? ']]>'       : '') . "\n"
	 . "  </script>\n";
     }

    self::session_start();
    /* everything that needs sessions started to work: */

    if (empty($options['school']))
      $options['school'] = school_load_guess();
    $this->school = $options['school'];
    if ($this->school['id'] != 'default')
      /* If we have a non-generic school, put it into base_title */
      array_unshift($this->base_title, $this->school['name']);

    if (empty($options['semester']))
      $options['semester'] = school_semester_guess($this->school);
    $this->semester = $options['semester'];

    if (!isset($options['school_semester_constant']))
      $options['school_semester_constant'] = TRUE;
    $this->school_semester_constant = (bool)$options['school_semester_constant'];

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
   * \param $options
   *   An array containing any of the following keys:
   *     - 'school': The school to use instead of the autodetected one.
   *     - 'semester': The semester to use instead of the autodetected one.
   *     - 'school_semester_constant': Whether the options to change
   *        the current school and semester should be hidden. TRUE by
   *        default.
   */
  public static function page_create($title, array $scripts = array(), array $options = array())
  {
    return new page(htmlentities($title, ENT_QUOTES, 'UTF-8'), $scripts, FALSE, $options);
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
   *   Declare that this page is a conversion point.
   *
   * Making a page a conversion point informs any ad services or
   * whatnot that the user made it this far in slate_permutate. If the
   * user was referred to slate_permutate via an advertisement, this
   * can be used to see whether a click actually resulted in the user
   * actually _using_ slate_permutate instead of just navigating away
   * upon reading the first page.
   */
  public function conversion()
  {
    global $ga_conversions;

    if (!empty($ga_conversions))
      {
	if (!empty($this->ga_conversions_code))
	  /* Function already called once. */
	  return;

	$conversion_base_href = 'http' . ($_SERVER['SERVER_PORT'] == 80 ? '' : 's') . '://www.googleadservices/pagead/conversion/';
	$conversion_hrefs = array();
	$conversion_referrer = empty($_SERVER['HTTP_REFERER']) ? '' : '&ref=' . rawurlencode(substr($_SERVER['HTTP_REFERER'], 0, 255));
	$js_Date_getTime = (1000 * time()) . sprintf("%03d", rand(0, 999));

	$i = 1;
	foreach ($ga_conversions as $conversion_id => $conversion_label)
	  /*
	   * For random, supplement time() with three numerals to look
	   * like milliseconds like JavaScript's Date.getTime()
	   * function. For some reason, `random' and `fst' (first
	   * conversion time?) are both set to the current time. I'm
	   * guessing that random is supposed to be a cachebreaker.
	   *
	   * `cv' is the `current version' of the Google conversion.js
	   * which is 7. This could be scraped from the .js by looking
	   * for `google_conversion_js_version="7"'.
	   *
	   * `fmt=3' must mean that we don't want the user-notification
	   * to appear, but we already don't show that.  `value=0'
	   * seems to have no meaning at all, maybe it is supposed to
	   * be the `priority' of this conversion point.
	   *
	   * Google's `hl' and `gl' language values should probably be
	   * appended.
	   */
	  $this->ga_conversions_code .= '<img alt="" style="width: 1px; height: 1px; border: none;" src="'
	  . htmlentities($conversion_base_href . $conversion_id . '/?random=' . $js_Date_getTime . '&cv=7&fst=' . $js_Date_getTime
			 . '&num=' . $i++ . '&fmt=3&value=0&label=' . $conversion_label . '&bg=ffffff'
			 . '&guid=ON&disvt=&is_call=' . $conversion_referrer,
			 ENT_QUOTES)
	  . '" ' . ($this->xhtml ? '/' : '') . '>';
      }
  }

  /**
   * \brief
   *   Set a meta element value.
   * \param $name
   *   The name of the meta attribute.
   * \param $value
   *   The value.
   */
  public function meta($name, $value = '')
  {
    $this->meta[$name] = $value;
  }

  /**
   * \brief
   *   Set the information necessary to create a canonical URI
   *   description.
   *
   * For declaring a page's canonical URI, we use both <link
   * rel="canonical"/> and soft redirects.
   *
   * \param $uri
   *   The base URI for the current page.
   * \param $query
   *   The querystring to canonicalize on.
   */
  public function canonize($uri, array $query = array())
  {
    $query_string = '';
    $uri_full = $uri;
    if (!empty($query))
      {
	ksort($query);
	$uri_full .= self::query_string($query);
      }

    /* Detect if we are at the canonical location or not... */
    list($base_request_uri) = explode('?', $_SERVER['REQUEST_URI'], 2);
    $base_request_uri = substr($_SERVER['REQUEST_URI'], strrpos($base_request_uri, '/') + 1);
    if ($base_request_uri != $uri_full)
      /* We are not canonical, redirect. */
      $this->redirect($uri_full);

    /* Mention that this is a canonical URI with <link rel="canonical"/> */
    $this->headcode_add('link_rel_canonical', '<link rel="canonical" href="'
			. htmlentities(self::uri_resolve($uri_full), ENT_QUOTES) . '"'
			. ($this->xhtml ? '/>' : '></link>'),
			TRUE);
  }

  /**
   * \brief
   *   Output the HTML header for a page, including <!DOCTYPE>, <head />, and opening structure
   */
  public function head()
  {
    global $cache_buster;

    if ($this->xhtml) {
       header('Content-Type: application/xhtml+xml; charset=utf-8');
      echo '<?xml version="1.0" encoding="utf-8"?>' . PHP_EOL;
    }
    else
      header('Content-Type: text/html; charset=utf-8');

    $cb = htmlentities('?_=' . rawurlencode(empty($cache_buster) ? '' : $cache_buster), ENT_QUOTES);
    echo '<!DOCTYPE ' . $this->doctype . '>'. PHP_EOL .
	  '<html ' . $this->htmlargs . '>'. PHP_EOL .
	  '  <head>'. PHP_EOL .
	  '    <title>' . $this->pagetitle . ' - ' . $this->base_title[0] . ' - ' . $this->base_title[1] . '</title>'. PHP_EOL .
          '    <link rel="stylesheet" href="styles/general.css' . $cb . '" type="text/css" media="screen" charset="utf-8" />'.  PHP_EOL .
	  '    <link rel="stylesheet" type="text/css" media="print" href="styles/print.css' . $cb . '" />'. PHP_EOL .
          '    <!--[if IE]>'. PHP_EOL .
          '      <link rel="stylesheet" type="text/css" media="screen" charset="utf-8" href="styles/ie.css' . $cb . '" />'. PHP_EOL .
          '    <![endif]-->'. PHP_EOL .
          '    <link rel="icon" href="images/favicon.svg" type="image/svg+xml" sizes="any" />' . PHP_EOL
      . '    <link rel="apple-touch-icon" href="images/favicon.svg" type="image/svg+xml" sizes="any" />' . PHP_EOL
      . '    <style type="text/css">' . PHP_EOL
      . $this->cdata_wrap(school_page_css($this->school))
      . '    </style>' . PHP_EOL;

    foreach ($this->meta as $key => $value)
      echo '    <meta name="' . htmlentities($key, ENT_QUOTES)
      . '" content="' . htmlentities($value, ENT_QUOTES)
      . '" ' . ($this->xhtml ? '/' : '') .  '>' . PHP_EOL;

    // Write out all passed scripts
    foreach ($this->scripts as $i)
      echo '    ' . $this->headCode["$i"] . "\n";

    /*
     * Perhaps we should have a separate array for javascript library
     * initialization snippets.
     */
    $javascript_init = '';
    if (in_array('jQuery.cuteTime', $this->scripts))
      $javascript_init .= ''
	. '// Support cuteTime failing to load (while js.ohnopub.net is fixed to be accepted by Chrome)' . PHP_EOL
	. 'if (jQuery.fn.cuteTime) {' . PHP_EOL
	. '  jQuery.extend(jQuery.fn.cuteTime.settings, {refresh: 10000, use_html_attribute: false});' . PHP_EOL
	. '  jQuery.fn.cuteTime.settings.time_ranges[0].cuteness = \'in the future\';' . PHP_EOL
	. '}' . PHP_EOL
	;

    echo $this->script_wrap(''
			    . 'var slate_permutate_school = ' . json_encode($this->school['id']) . ';' . PHP_EOL
			    . 'var slate_permutate_semester = ' . json_encode($this->semester['id']) . ';' . PHP_EOL
			    . $javascript_init) . PHP_EOL;

    $selectschool_query = '&amp;school=' . $this->school['id'];
    /* kludge */
    if (!empty($_REQUEST['s']))
      $selectschool_query .= '&amp;s=' . (int)$_REQUEST['s'];

    $logo_html = !$this->title_uses_svg ? '<span style="font-size: ' . $this->title_font_size_multiplier . 'em;"><span style="color: #777777">' . $this->slate_t . '</span><span style="color: #000000">' . $this->permutate_t . '</span></span>' : '<img src="images/slatepermutate-alpha.svg" alt="SlatePermutate" class="noborder" />';

    echo
      '  </head>' . PHP_EOL .
	 '  <body class="desired-lang-' . $this->desired_lang . '">'. PHP_EOL .
         '    <div id="page">'. PHP_EOL .
         '      <div id="header">'. PHP_EOL .
	 '        <div id="title">'. PHP_EOL .
      '          <h1 id="logo-heading"><a style="text-decoration: none;" href="' . htmlentities($this->uri_resolve(), ENT_QUOTES) . '">' . $logo_html . '</a></h1>'. PHP_EOL .
         '          <p>'. PHP_EOL .
         '            <span id="subtitle">'.$this->pagetitle.'</span>'. PHP_EOL .
  	 '            <span id="menu">' . PHP_EOL
       . '              <em>' . $this->school['name'] . '</em>' . ($this->school_semester_constant ? '' : ' <a href="input.cgi?selectschool=1' . $selectschool_query . '" title="Choose a different school">(change)</a>') . PHP_EOL;
    if (!empty($this->semester))
      echo  '             <em>' . $this->semester['name'] . '</em>' . ($this->school_semester_constant ? '' : ' <a href="input.cgi?selectsemester=1' . $selectschool_query . '" title="Choose a different semester">(change)</a>') . PHP_EOL;
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
  	 '        <div id="leftfoot">'. PHP_EOL .
	 '          <div><a href="feedback.cgi">Help / Contact Us</a></div>'. PHP_EOL .
         '          <div><a href="about.cgi">About</a></div>' . PHP_EOL .
         '        </div>'. PHP_EOL .
         '        <div id="rightfoot">'. PHP_EOL .
         '          <div><em>© <a href="https://www.facebook.com/ngelderloos/">Nathan Gelderloos</a><br /><a href="http://ethanzonca.com">Ethan Zonca</a><br /><a href="https://twitter.com/ohnobinki">Nathan Phillip Brink</a></em></div>'. PHP_EOL .
	 '        </div>'. PHP_EOL .
      $this->ga_conversions_code . PHP_EOL .
         '      </div> <!-- id="footer" -->'. PHP_EOL .
         '    </div> <!-- id="page" -->'. PHP_EOL;
    echo $this->trackingcode;
    echo '  </body>'. PHP_EOL .
         '</html>' . PHP_EOL;
    $this->pageGenTime = round(microtime(TRUE) - $this->pageGenTime,4);
    echo '<!-- Page generated in ' . $this->pageGenTime . ' seconds -->' . PHP_EOL;

  }

  /**
   * \brief
   *   Shows a box with recently processed schedules
   */
  public function showSavedScheds($session) {
    global $clean_urls;

    if (isset($session['saved']) && count($session['saved']) > 0) {
      echo '<div id="savedBox" class="note saved">' . PHP_EOL;

      $process_php_s = 'process.cgi?s=';
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
	    . ' <a href="input.cgi?s=' . $key . '">edit</a>'
	    . ' <a href="process.cgi?del=' . $key . '">delete</a>'
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
  public function showSemesters($linkto = 'input.cgi')
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
    /**
     * Show the historical data. This doesn't seem useful, but it's here.
     */
    $historical = FALSE;

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
	if ($semester['time_start'] > 36000 && $semester['time_start'] < ($time - 365*24*60*60))
	  {
	    $class_extra .= ' historical';
	    $historical = TRUE;
	  }

	if ($semester['time_start'] < $time && $semester['time_end'] > $time)
	  $text_extra[] = 'current';

	$text_extra = implode(', ', $text_extra);
	if (strlen($text_extra))
	  $text_extra = ' (' . $text_extra . ')';

	echo '        <li class="semester' . $class_extra . '"><a href="' . $linkto . $semester['id'] . '">' . htmlentities($semester['name']) . '</a>' . $text_extra . "</li>\n";
      }
    if ($historical)
	echo '       <li style="display: none;" class="historical-show"><a class="historical-show-a" href="#">(Show historical semesters…)</a></li>' . PHP_EOL;
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
   *
   * \param $cache_limiter
   *   Specify the sort of session-related cache limitation is used,
   *   see session_cache_limiter().
   */
  public static function session_start($cache_limiter = 'nocache', $cache_expire = NULL)
  {
    static $session_started = FALSE;

    if (!$session_started)
      {
	session_cache_limiter($cache_limiter);
	if ($cache_expire !== NULL)
	  session_cache_expire($cache_expire);
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
   *   'input.cgi' or '44' (for clean urls, for example).
   * \param $http_code
   *   The redirection code to use, if any. For example, this can be
   *   used to implement ``permanent'' redirects if necessary.
   */
  public static function redirect($dest, $http_code = NULL)
  {
    if ($http_code)
      /**
       * \todo
       *   See http://drupal.org/node/208793
       */
      header('HTTP/1.1 ' . $http_code);

    header('Location: ' . self::uri_resolve($dest));
    header('Vary: Accept-Language');
    exit();
  }

  /**
   * \brief
   *   Calculate the absolute URI on a best-effort basis.
   * \param $uri
   *   The relative URI. An empty string will get the URI to the
   *   index/default page.
   * \return
   *   An absolute URI referring to the specified page.
   */
  public static function uri_resolve($uri = '')
  {
    global $base_uri;

    static $host = '';
    if (empty($host))
      {
	if (isset($_SERVER['SERVER_NAME']))
	  $host = $_SERVER['SERVER_NAME'];
	if (isset($_SERvER['HTTP_HOST']))
	  $host = $_SERVER['HTTP_HOST'];
      }

    if (empty($base_uri))
      if (strlen($host))
	{
	  $proto = 'http';
	  $port = NULL;
	  if (isset($_SERVER['SERVER_PORT']) && $_SERVER['SERVER_PORT'] != 80)
	    {
	      if ($_SERVER['SERVER_PORT'] == 443
		  || (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off'))
		$proto .= 's';
	      if ($_SERVER['SERVER_PORT'] != 443)
		$port = $_SERVER['SERVER_PORT'];
	    }
	  
	  $base_uri = $proto . '://' . $host;
	  if ($port !== NULL)
	    $base_uri .= ':' . $port;
	  list($base_request_uri) = explode('?', $_SERVER['REQUEST_URI'], 2);
	  $base_uri .= rtrim(substr($base_request_uri, 0, strrpos($base_request_uri, '/')), '/') . '/';
	}

    if (empty($base_uri) && empty($uri))
      return './';

    return $base_uri . $uri;
  }

  /**
   * \brief
   *   Resolve an SSL address for a static asset.
   *
   * This is pretty much a hack in support of another hack. I need to
   * provide some assets over SSL; if the local server doesn’t support
   * that properly (such as by not having a properly signed SSL
   * certificate), a web-storage backend can be used instead. This can
   * only be used with static content.
   *
   * \param $uri
   *   The path to a static file which needs to be served over SSL.
   */
  public static function uri_resolve_sslasset($uri, $type)
  {
    global $s3_bucket, $s3_accesskey, $s3_secretkey;

    $testuri = page::uri_resolve($uri);
    if (!strncmp($testuri, 'https://', strlen('https://')))
      /*
       * The user is already accessing this page as SSL, so serving
       * another asset over the same channel will not appear any less
       * trusted to the user.
       */
      return $testuri;

    /*
     * Use an external service if configured.
     */
    if (!empty($s3_bucket) && !empty($s3_accesskey) && !empty($s3_secretkey))
      {
	/*
	 * Load S3 cache.
	 */
	$dirpath = dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR;
	$s3_cache_path = $dirpath . 'saved_schedules' . DIRECTORY_SEPARATOR . '.s3_cache';
	$s3_cache = @unserialize(file_get_contents($s3_cache_path));
	if (empty($s3_cache))
	  $s3_cache = array();

	$path = $dirpath . $uri;
	$sha1 = sha1_file($path);

	if (empty($s3_cache[$sha1]))
	  {
	    @include 'S3.php';
	    if (class_exists('S3'))
	      {
		$s3 = new S3($s3_accesskey, $s3_secretkey);
		$bucket = $s3->getBucket($s3_bucket);
		if ($bucket === FALSE)
		  $bucket = $s3->putBucket($s3_bucket, S3::ACL_PUBLIC_READ);
		if ($bucket !== FALSE)
		  if ($s3->putObject(S3::inputFile($path), $s3_bucket, $sha1, S3::ACL_PUBLIC_READ, array(), array('Content-Type' => $type)))
		    {
		      $s3_cache[$sha1]['uri'] = 'https://' . $s3_bucket . '.s3.amazonaws.com/' . $sha1;
		      file_put_contents($s3_cache_path, serialize($s3_cache), LOCK_EX);
		    }
	      }
	  }
	if (!empty($s3_cache[$sha1]['uri']))
	  return $s3_cache[$sha1]['uri'];
      }

    if (!strncmp($testuri, 'http://', strlen('http://')))
      {
	/* Test if we can create a local HTTPS connection… */
	$curl = curl_init();
	curl_setopt($curl, CURLOPT_USERAGENT, SP_PACKAGE_NAME . '/' . SP_PACKAGE_VERSION);
	$testuri2 = 'https' . substr($testuri, strlen('https'));
	curl_setopt($curl, CURLOPT_URL, $testuri2);
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
	$result = curl_exec($curl);
	curl_close($curl);
	if (!empty($result) && sha1($result) === $sha1)
	  return $testuri2;
      }
    return $testuri;
  }

  /**
   * \brief
   *   Return an array of name=value pairs that are urlencoded.
   *
   * Supports query_string() and query_formbutton().
   */
  private static function _uriencode_query_array(array $query)
  {
    $query_string_parts = array();
    foreach ($query as $param => $values)
      {
	if (!is_array($values))
	  $values = array($values);
	foreach ($values as $value)
	  $query_string_parts[] = rawurlencode($param) . '=' . rawurlencode($value);
      }
    return $query_string_parts;
  }

  /**
   * \brief
   *   Form a query string from a map.
   *
   * \param $query
   *   The map of keys onto values to form into a querystring.
   * \param $question
   *   Include the question mark which delimits the querystring in a
   *   URI.
   * \return
   *   A querystring suitable for appending to a URI. Includes the `?'
   *   by default.
   */
  public static function query_string(array $query, $question = TRUE)
  {
    $query_string_parts = self::_uriencode_query_array($query);
    if (count($query_string_parts))
      return ($question ? '?' : '') . implode('&', $query_string_parts);
    return '';
  }

  /**
   * \brief
   *   Return an HTML form button which submits all keys, as many of
   *   them with GET as possible.
   *
   * Allows one to automatically delegate fatter values to be POSTed
   * to prevent the querystring from getting too long and making the
   * URI itself become too long. Always returns a <form/> with a
   * <button/>. The <form/> may be method="GET", unless there is too
   * much data in which case it becomes method="POST".
   *
   * Currently, this function will mess up the order of parameters. If
   * order matters, this function will not work for you.
   *
   * \sa query_string()
   *   An alternative to calling query_string() when unbounded amounts
   *   of data may need to be transmitted.
   *
   * \param $uri
   *   The URI to submit the data to. Will be used as-is.
   * \param $query
   *   The map of parameters onto values.
   * \param $button_html
   *   A valid XHTML fragment to place inside of the button, such as
   *   page::entities($text) telling the user what the button does.
   * \param $button_pre_html
   *   The HTML which wraps around the <button/>, such as the opening
   *   of a <p/> within which a <button/> may be placed.
   * \param $button_post_html
   *   The close of the HTML wrapping around the <button/>, such as
   *   the closing of a <p/>.
   */
  public function query_formbutton($uri, array $query, $button_html, $button_pre_html, $button_post_html)
  {
    /*
     * Recommended, but low, upper URI limit. Modern browsers can
     * handle around 2000+ chars, so could be upped to 2000 without
     * harm probably.
     */
    $uri_len_limit=255;

    /*
     * Calculate urlencoded lengths of param/values so as to greedily
     * take the smallest params into GET…
     */
    $uriencoded_parts = self::_uriencode_query_array($query);

    /*
     * Join parameters of the same name together…
     */
    $flirting_uriencoded_parts = array();
    foreach ($uriencoded_parts as $uriencoded_part)
      {
	list($key) = explode('=', $uriencoded_part);
	if (empty($flirting_uriencoded_parts[$key]))
	  $flirting_uriencoded_parts[$key] = $uriencoded_part;
	else
	  $flirting_uriencoded_parts[$key] .= '&' . $uriencoded_part;
      }

    usort($flirting_uriencoded_parts, function($a, $b) {
	$a_strlen = strlen($a);
	$b_strlen = strlen($b);
	/*
	 * There is no “ursort()”, so reverse the sort so that
	 * shortest is first.
	 */
	return $a_strlen > $b_strlen ? 1 : ($a_strlen == $b_strlen ? 0 : -1);
      });

    $uri_orig = $uri;
    $query_orig = $query;
    if (strpos($uri, '?') === FALSE
	&& !empty($uriencoded_parts))
      $uri .= '?';
    $uri_len = strlen($uri);

    $first = TRUE;
    foreach ($flirting_uriencoded_parts as $last => $flirting_uriencoded_part)
      {
	if (($new_uri_len = ($first ? 0 : 1) + $uri_len + strlen($flirting_uriencoded_part)) > $uri_len_limit)
	  break;
	if ($first)
	  $first = FALSE;
	else
	  $uri .= '&';

	$uri .= $flirting_uriencoded_part;
	$uri_len = $new_uri_len;

	/*
	 * Drop this param from the $query array as we have taken care
	 * of it and don’t need to have it be in POST.
	 */
	list($key) = explode('=', $flirting_uriencoded_part);
	$key = rawurldecode($key);
	unset($query[$key]);
      }

    if (empty($query))
      {
	$method = 'get';
	/*
	 * When making a <form method="get"/>, the browser will clear
	 * out the entire querystring portion of the URI. Thus, we
	 * need to reformat everything as <input/>… We can only have
	 * some things in action="" as GET params if our form is POST.
	 */
	$query = $query_orig;
	$uri = $uri_orig;
      }
    else
      $method = 'post';
    $form = '<form method="' . self::entities($method) . '" action="' . self::entities($uri) . '">' . PHP_EOL;
    foreach ($query as $key => $values)
      {
	if (!is_array($values))
	  $values = array($values);
	foreach ($values as $value)
	  $form .= '  <input type="hidden" name="' . self::entities($key) . '" value="' . self::entities($value) . '" ' . $this->element_self_close() . '>' . PHP_EOL;
      }
    return ''
      . $form . '  ' . $button_pre_html . '<button type="submit">' . $button_html . '</button>' . $button_post_html . PHP_EOL
      . '</form>';
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
   *   Get the current semester.
   */
  public function semester_get()
  {
    return $this->semester;
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
    return ''
      . '<script type="' . $type . '">' . PHP_EOL
      . $this->cdata_wrap($js)
      . '// </script>';
  }

  /**
   * \brief
   *   Wrap something in CDATA or not wrap depending on if we're
   *   serving HTML.
   *
   * Lower-level than Page::script_wrap().
   * \param $content
   *   The stuff to wrap in CDATA.
   * \return
   *   The wrapped string.
   */
  public function cdata_wrap($content)
  {
    return ''
      . ($this->xhtml ? '<![CDATA[' : '') . PHP_EOL
      . $content . PHP_EOL
      . ($this->xhtml ? ']]>' : '') . PHP_EOL;
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
      return 'http' . ($_SERVER['SERVER_PORT'] == 80 ? '' : 's') . '://' . $_SERVER['HTTP_HOST'] . $this->add_trailing_slash(dirname($_SERVER['REQUEST_URI'])) . '' . $id;
    }
    else {
      return 'http' . ($_SERVER['SERVER_PORT'] == 80 ? '' : 's') . '://' . $_SERVER['HTTP_HOST']  . $this->add_trailing_slash(dirname($_SERVER['REQUEST_URI'])) . 'process.cgi?s=' . $id;
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

  /**
   * \brief
   *   Encode things using htmlentities() with proper precautions.
   */
  public static function entities($text)
  {
    $opts = ENT_QUOTES;
    if (defined('ENT_XML1'))
      $opts |= ENT_XML1;
    return htmlentities($text, $opts, 'utf-8');
  }
}
