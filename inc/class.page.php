<?php

class page {

// Private functions/variables

  private $base_title = 'SlatePermutate';
  private $doctype = 'html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';
  private $htmlargs = 'xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"';
  private $bodyargs = '';
  public $lastJobTable = '';

  private $isMobile = false; // Have a basic view when this is true

  private $pageGenTime = 0;

  private $indexpath = 'http://protofusion.org/SlatePermutate/'; // full url to index for php header redirection

  private $trackingcode = '<script type="text/javascript">
				  var _gaq = _gaq || [];
				  _gaq.push([\'_setAccount\', \'UA-17441156-1\']);
				  _gaq.push([\'_trackPageview\']);
				  (function() {
				    var ga = document.createElement(\'script\'); ga.type = \'text/javascript\'; ga.async = true;
				    ga.src = (\'https:\' == document.location.protocol ? \'https://ssl\' : \'http://www\') + \'.google-analytics.com/ga.js\';
				    var s = document.getElementsByTagName(\'script\')[0]; s.parentNode.insertBefore(ga, s);
				  })();
				</script>'; // Google analytics ga.js tracking code

  public $tablestripe = '<script type="text/javascript">
			  $(document).ready(function(){
			    $(".tablestripe tr").mouseover(function() {$(this).addClass("over");}).mouseout(function() {$(this).removeClass("over");});
			    $(".tablestripe tr:even").addClass("alt");
			    $(".tablestripe tr:odd").addClass("alto");
			  });
			  </script>';

  private $title = ''; // Title of page
  private $scripts = ''; // Scripts to include on page

  public function __construct($ntitle, $nscripts = ''){
    $this->title = $ntitle;
    $this->scripts .= $nscripts;
    if($ntitle != "NOHEAD")
      $this->head();
  }

  private function top(){
    echo '<div id="header">
          <h1><em>SlatePermutate</em> -  Scheduler</h1>
          </div>
          <div id="content">';
  }

// Public functions/vars

  public function head(){
    session_start();

    // @TODO: Add capability to load additional javascripts and stylesheets in array form

    $this->pageGenTime = round(microtime(), 3);

    echo '<!DOCTYPE ' . $this->doctype . '>
	  <html ' . $this->htmlargs . '>
	  <head>
	    <title>' . $this->title . ' :: ' . $this->base_title . '</title>

         <link rel="stylesheet" href="styles/general.css" type="text/css" media="screen" charset="utf-8">';

	echo $this->scripts;
	echo '</head>
	  <body '.$this->bodyargs.' >';
    echo $this->top(); // Write out top
  }


  public function foot(){
    echo '</div>';
    $this->pageGenTime = round(microtime(), 3);
    echo '<div id="footer"><h5>&copy; '. date('Y').' <a href="http://protofusion.org/~nathang/">Nathan Gelderloos</a><br />
                           with special thanks to <a href="http://ethanzonca.com">Ethan Zonca</a></h5>
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

}

?>