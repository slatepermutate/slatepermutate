<?php /* -*- mode: php; -*- */
/*
 * Copyright 2012 Nathan Phillip Brink
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

require_once('inc/class.page.php');

$page = page::page_create('WebAdvisor');
$school = $page->get_school();

if (empty($school['webadvisor_url']))
  {
    if (!empty($school['registration_url']) && preg_match(',(.*/WebAdvisor),', $school['registration_url'], $matches))
      $school['webadvisor_url'] = $matches[1];
    else
      $school['webadvisor_url'] = $school['url'] . 'WebAdvisor';
  }

/**
 * \brief
 *   Calculate the URI necessary for logging into WebAdvisor.
 *
 * \param $school
 *   The school.
 * \param $dest
 *   The URI to visit after the user has logged into WebAdvisor and
 *   the TOKENIDX has been communicated to $tokenidx_callback.
 * \param $tokenidx_callback
 *   A JSONP-compatible callback which must be passed the TOKENIDX
 *   parameter the WebAdvisor is using. Treat as if is terminated with
 *   a `?' -- i.e., just append the querystring without the `?' to
 *   this URI when constructing the callback. To use, for example, in
 *   JavaScript you may create a DOMElement 'script' with attributes
 *   type="text/javascript" and
 *   src="$tokenidx_callback?callback=jsonp_callback&TOKENIDX=<detected
 *   TOKENIDX>". When jsonp_callback gets called, your script knows
 *   that $dest may be returned to. Don't forget to allow the user to
 *   log in first. This is normally done by setting SP_CALLBACK GET
 *   variable to this value inserting the
 *   scripts/webadvisor_tokenidx.js script into the WebAdvisor login
 *   page using cross-site-scripting HTML injection such as through
 *   the ERROR GET parameter.
 * \return
 *   Just ensure that $tokenidx_callback gets called; do not return
 *   except by redirecting to $dest.
 */
function webadvisor_login($page, array $school, $dest, $tokenidx_callback)
{
  if (strpos($dest, '?') !== FALSE)
    $dest .= '&';
  else
    $dest .= '?';
  $dest .= 'from_webadvisor';

  $webadvisor_login_func = $school['id'] . '_webadvisor_login';
  if (function_exists($webadvisor_login_func))
    $webadvisor_login_func($school, $dest);

  /*
   * The hack we are using is that somehow TOKENIDX=&SS=LGRQ&URL=<URI>
   * will both initialize the user's browser with a token cookie and
   * then redirect to URL. Trying to use the proper way of loading the
   * LGRQ (using TYPE=P&PID=UT-LGRQ&PROCESS=-XUTAUTH01) doesn't work
   * because it drops and ignores our URL parameter, leaving the user
   * at the KV site. No other URL I've fiddled with seems to be able
   * to do this combination of logging in and returning the user to us
   * or a URI of our choosing. Once the user's browser has been
   * initialized with a TOKENIDX, loading the page
   * SS=LGRQ&URL=<URI>&ERROR=<XSS> will preserve the ERROR=<XSS>
   * necessary for our XSS and insert it into the login page.
   *
   * HOWEVER, if the browser already has a TOKENIDX-related cookie,
   * then visiting TOKENIDX=&SS=LGRQ&URL=<URL> will cause WebAdvisor
   * to keep redirecting to itself infinitely. Similarly, if the
   * browser does not yet have a TOKENIDX-related cookie,
   * SS=LGRQ&URL=<URL> will redirect the user to URL without giving
   * the user a cookie. Thus, our strategy is to specify
   * LASTTOKEN=NULL: this is the magic which prevents the silly
   * infinite looping when the user already has cookies and also works
   * when the user doesn’t have a cookie to start with.
   */

  $login_form_uri = $school['webadvisor_url'] . '?LASTTOKEN=NULL&SS=LGRQ&URL=' . rawurlencode($dest)
    . '&SP_CALLBACK=' . rawurlencode($tokenidx_callback)
    . '&ERROR=' . rawurlencode('<script type="text/javascript" src="' . htmlentities(page::uri_resolve_sslasset('scripts/webadvisor_tokenidx.js', 'text/javascript'), ENT_QUOTES) . '"></script><span id="sp_err">Slate Permutate loading… (automatic registration may not be working)</span>');
  redir($login_form_uri);
}

function redir($dest)
{
  header('HTTP/1.1 302 Found');
  header('Location: ' . $dest);
  header('Content-Type: text/plain; charset=utf-8');
  echo 'Location: ' . $dest;
  exit;
}

if (empty($_GET['TOKENIDX']))
  {
    /*
     * Get a token for the ST-WERG form and have the user perform the
     * WebAdvisor-specific login. This can only be done after the
     * login form has an SS allocated for it.
     */
    webadvisor_login($page, $school, page::uri_resolve('webadvisor.php') . '?r=' . rand()
		     . '&sections=' . rawurlencode(empty($_GET['sections']) ? '' : $_GET['sections'])
		     . '&school=' . rawurlencode($school['id']),
		     page::uri_resolve('webadvisor.php?'));
  }

/*
 * Use the hopefully-still-valid TOKENIDX to initialize an ST-WERG
 * (STudent Web[A]dvisor Express ReGistration) form. When that form is
 * iniailized, assume that it has SS=1 and submit the form. &APP=ST
 */
$TOKENIDX = $_GET['TOKENIDX'];
$page->head();
echo '<form id="sp-webadvisor-form" action="' . htmlentities($school['webadvisor_url'] . '?TOKENIDX=' . $TOKENIDX . '&SS=1', ENT_QUOTES) . '" method="post">' . PHP_EOL;
echo '<p>';

$uri = $school['webadvisor_url'] . '?TOKENIDX=' . $TOKENIDX . '&TYPE=P&PID=ST-WERG';
$onload_html = '="' . htmlentities('javascript:document.getElementById(\'sp-webadvisor-form\').submit()', ENT_QUOTES) . '"';
echo '  <img src="' . htmlentities($uri, ENT_QUOTES) . '" alt="Loading WebAdvisor Express Registration form (ST-WERG)…"'
. ' onload' . $onload_html . ' onerror' . $onload_html . ' />' . PHP_EOL;
echo '  If you are not redirected after 16 seconds, you may try: ' . PHP_EOL;

$sections = explode(',', empty($_GET['sections']) ? '' : $_GET['sections']);
echo '  <input type="hidden" name="LIST.VAR1_CONTROLLER" value="LIST.VAR1" />' . PHP_EOL;
echo '  <input type="hidden" name="LIST.VAR1_MEMBERS" value="LIST.VAR1*LIST.VAR2*LIST.VAR3*LIST.VAR4*LIST.VAR5" />' . PHP_EOL;
for ($i = 1; $i <= 5; $i ++)
  echo //'  <input type="hidden" name="LIST.VAR' . $i . '_MAX" value="' . count($sections) . '" />' . PHP_EOL;
    '  <input type="hidden" name="LIST.VAR' . $i . '_MAX" value="10" />' . PHP_EOL;
$course_num = 1;
foreach ($sections as $course)
  {
    echo '  <input type="hidden" name="LIST.VAR1_' . $course_num . '" value="' . htmlentities($course, ENT_QUOTES) . '" />' . PHP_EOL;
    for ($i = 2; $i <= 5; $i ++)
      echo '  <input type="hidden" name="LIST.VAR' . $i . '_' . $course_num . '" value="" />' . PHP_EOL;
    $course_num ++;
  }
while ($course_num < 10)
  {
    for ($i = 1; $i <= 5; $i ++)
      echo '  <input type="hidden" name="LIST.VAR' . $i . '_' . $course_num . '" value="" />' . PHP_EOL;
    $course_num ++;
  }      
echo '  <input type="hidden" name="SUBMIT_OPTIONS" value="" title="Please do not click here unless if you are not redirected within 16 or more seconds." />' . PHP_EOL;
echo '  <input type="submit" name="SUBMIT2" value="SUBMIT" />' . PHP_EOL;
echo '</p>';
echo '</form>';

$page->foot();
