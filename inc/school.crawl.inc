<?php /* -*- mode: php; -*- */
/*
 * Copyright 2010 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
 *
 * This file is a part of slate_permutate.
 *
 * slate_permutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * slate_permutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with slate_permutate.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 *   Routines that are only useful when crawling schools' websites for
 *   autofill section data.
 */

/**
 * \brief
 *   Initialize a school_crawl_log handle.
 *
 * \param $school
 *   The school for which this crawl handle is.
 * \param $opts
 *   An array optionally with one of the following keys:
 *   - stream: an fopen()-compatible stream to fwrite()/fprintf() output to.
 *   - page: a Page object used to help format HTML output.
 *   - verbosity: A number from 0 through 10 describing the desired
 *       verbosity.
 */
function school_crawl_log_init(array $school, $opts = array())
{
  $opts += array('verbosity' => 5);
  return array('school' => $school, 'out' => array('html' => array(), 'plain' => array())) + $opts;
}

/**
 * \brief
 *   Log progress of a crawler.
 *
 * This function's arguments take the same style as fprintf() does.
 *
 * \param $school_crawl_log
 *   The logging resource.
 * \param $verboseness
 *   The verbosity level at which to log the message. Should be a
 *   value from 0 to 10, where 0 is unconditionally printed and 5 is
 *   the default.
 * \param $format
 *   The printf()-style format string.
 */
function school_crawl_logf(array $school_crawl_log, $verboseness, $format)
{
  $args = func_get_args();
  array_shift($args);
  array_shift($args);

  if ($verboseness > $school_crawl_log['verbosity'])
    /*
     * The given message gives us more detail than we want. Therefore,
     * discard it.
     */
    return;

  $log_line = call_user_func_array('sprintf', $args);

  /* store output in a place where it's retrievable */
  $school_crawl_log['out']['plain'][] = sprintf("%s_crawl(): %s\n",
						$school_crawl_log['school']['id'], $log_line);

  /* store the output in a retrievable list of outputs */
  if (isset($school_crawl_log['page']))
    $school_crawl_log['out']['html'][] = sprintf("<div class=\"logline\"><tt>%s_crawl()</tt>: %s</div><br class=\"logline\"%s>\n",
						 $school_crawl_log['school']['id'], htmlentities($log_line),
						 $school_crawl_log['page']->element_self_close());

  /* print to a stream potentially */
  if (isset($school_crawl_log['stream']))
    fprintf($school_crawl_log['stream'], "%s_crawl(): %s\n", $school_crawl_log['school']['id'], $log_line);

  return 0;
}

/**
 * \brief
 *   Recover stored crawling log stuffage.
 *
 * \param $html
 *   Whether to retrieve formatted HTML output if it's available.
 * \return
 *   An array of output lines.
 */
function school_crawl_log_fetch(array $school_crawl_log, $html = FALSE)
{
  if ($html)
    if (isset($school_crawl_log['page']))
      return $school_crawl_log['out']['html'];
    else
      return nl2br(htmlentities($school_crawl_log['out']['plain']));
  return $school_crawl_log['out']['plain'];
}

/**
 * \brief
 *   Parse a simple time string into slate_permutate's time
 *   representation.
 *
 * \param $time
 *   An array compatible with the return value of strptime(). The only
 *   fields we use are 'tm_hour', which is from 0 through 23, and
 *   'tm_min', which may be from 0 through 50.
 */
function school_crawl_time_format($time)
{
  return sprintf('%02d%02d', $time['tm_hour'], $time['tm_min']);
}

/**
 * \brief
 *   Equivalent of gmmktime() except that it accepts strptime()'s
 *   output format as an input.
 *
 * \param $tm
 *   An array formatted as the output of strptime().
 * \param $timezone_offset
 *   Optional offset of the school's timezone in seconds from
 *   UTC. This offset gets _added_ to the resulting timestamp. So, for
 *   example, Eastern Daylight Time would use a value of 60*60 * -4
 *   since it is -0400 during Daylight time.
 * \return
 *   A unix timestamp.
 */
function school_crawl_gmmktime(array $tm, $timezone_offset = 0)
{
  return gmmktime($tm['tm_hour'], $tm['tm_min'], $tm['tm_sec'],
		$tm['tm_mon'] + 1, $tm['tm_mday'], $tm['tm_year'] + 1900)
    + $timezone_offset;
}

/**
 * \brief
 *   Take an array of day names and assemble them into
 *   slate_permutate's internal (weird) representation of a set of
 *   weekdays.
 *
 * This function is intended to make it easy for one to take the
 * output of an explode() call. For example, to decode $days_str =
 * 'Monday, Tuesday, Friday', one would do
 * school_crawl_days_format($school_crawl_log, explode(', ', $days_str));
 *
 * \param $school_crawl_log
 *   A school_crawl_log handle to report errors to.
 * \param $days
 *   An array of day names. These may be common abbreviations or
 *   truncations (any truncations must be two chars long for
 *   simplicity. One-char representations are supported, however, but
 *   use 'm', 't', 'w', 'h', 'f' to distinguish Thursday and
 *   Tuesday. 'r' may also be used for Thursday.). Case does not
 *   matter. 's' is for Saturday, based on CCBCMD. 'u' is for Sunday.
 * \return
 *   slate_permutate's strange internal days representation.
 */
function school_crawl_days_format(array $school_crawl_log, $days)
{
  static $daymap_1 = array('u' => 'u', 'm' => 'm', 't' => 't', 'w' => 'w', 'h' => 'h', 'r' => 'h', 'f' => 'f', 's' => 's');
  static $daymap_2 = array('su' => 'u', 'th' => 'h');

  $my_days = array();
  foreach ($days as $day)
    {
      $day_orig = $day;
      $day = strtolower(substr(trim($day), 0, 2));

      /*
       * convert from two-char representation to one-char
       * representation.n
       */
      if (strlen($day) > 1)
	{
	  if (isset($daymap_2[$day]))
	    $day = $daymap_2[$day];
	  else
	    $day = substr($day, 0, 1);
	}
      if (isset($daymap_1[$day]))
	$my_days[$daymap_1[$day]] = TRUE;
      else
	school_crawl_logf($school_crawl_log, 5, "school_crawl_days_format() got invalid day specifier: `%s' => `%s'.",
			  $day_orig, $day);
    }

  $day_str = '';
  foreach ($my_days as $day_val => $junk)
    $day_str .= $day_val;

  return $day_str;
}

/**
 * \brief
 *   Take a string of day initials and format it.
 *
 * \param $school_crawl_log
 *   The school_crawl_log handle to write errors out to.
 * \param $days_str
 *   Example input: 'mwf', 'TR'.
 * \return
 *   Same as school_crawl_days_format()
 */
function school_crawl_days_str_format(array $school_crawl_log, $days_str)
{
  $days_str = preg_replace('/[^[:alnum:]]+/', '', $days_str);

  /*
   * str_split() will produce an array with an empty string in it if
   * the input string is empty. We just want an empty array in that
   * case, but also if there are no input days we can just shortcut
   * and return no days.
   */
  if (empty($days_str))
    return '';

  return school_crawl_days_format($school_crawl_log, str_split($days_str));
}

/**
 * \brief
 *   Try to guess a more standardized section_meeting type.
 *
 * \param $meeting_type
 *   The upstream's meeting_type, such as 'LEC', 'lec', 'LAB',
 *   etc. New mappings should be added to this function as long as
 *   they are general enough.
 */
function school_crawl_meeting_type($meeting_type = 'lecture')
{
  static $meeting_type_maps =
    array(
	  'lec' => 'lecture',
	  'lab' => 'lab',
	  'dis' => 'discussion',
	  );

  if (empty($meeting_type))
    $meeting_type = 'lecture';

  $meeting_type = strtolower(trim($meeting_type));
  if (!empty($meeting_type_maps[$meeting_type]))
    $meeting_type = $meeting_type_maps[$meeting_type];
  elseif (!empty($meeting_type_maps[substr($meeting_type, 0, 3)]))
    $meeting_type = $meeting_type_maps[substr($meeting_type, 0, 3)];

  return $meeting_type;
}

/**
 * \brief
 *   Simulate some aspects of a web browser while retreiving a
 *   document.
 *
 * This allows us to view our cookies in an associative array and to
 * have the server's response automatically update our cookies.
 *
 * If $post is specified as an associative array, an HTTP POST is
 * performed and the data is encoded properly as if we were performing
 * a form submission.
 *
 * Follows redirects. If there is a redirect, the page from which you
 * are redirected is lost... but few people put any information on
 * those pages anyways ;-).
 *
 * \param $uri
 *   The URL to fetch. If a redirect occurs, this is updated.
 * \param $cookies
 *   An associative array of cookies and where to save new cookies.
 * \param $school_crawl_log
 *   The school_crawl_log handle to use.
 * \param $post
 *   If not NULL, causes an HTTP POST. In that case, should be an
 *   associative array of form keys/values.
 * \param $follow_meta_refresh
 *   Parse the resultant HTML with http://docs.php.net/dom and if it
 *   contains a line that looks like ``<meta http-equiv="Refresh" content="0; url=https://simon.ccbcmd.edu/pls/PROD/bwckschd.p_disp_dyn_sched">'',
 *   follow that URL.
 * \param $curlsetup_hook
 *   A function which is passed a curl handle which allows the caller
 *   to do silly things like setting CURLOPT_SSLVERSION for silly
 *   sites like ccbcmd's registration site.
 * \param $loopspin
 *   An internal variable to prevent us from following perpetual
 *   redirects.
 * \param $options
 *   Extra optional arguments with keys as follows:
 *   - 'writefunc': A curl-compatible write function of the form
 *       function($state, $data) and returns the number of eaten bytes
 *       which must be equal to the number of bytes received unless if
 *       the transfer should be aborted. Settings this and using
 *       $follow_meta_refresh are mutually exclusive and will cause
 *       undefined behavior.
 *   - 'writestate': The value which should be passed to writefunc as
 *       the $state parameter.
 * \return
 *   The body of the document returned by the server (normally
 *   malformed HTML, especially with Calvin's WebAdvisor installation)
 *   or, if 'writestate' and 'writefunc' are set, the value stored in
 *   'writestate'.
 */
function school_crawl_geturi(&$uri, &$cookies, array &$school_crawl_log, $post = NULL, $follow_meta_refresh = FALSE, $curlsetup_hook = NULL, $options = array(), $loopspin = 0)
{
  global $school_crawl_geturi_writefunc,
    $school_crawl_geturi_writestate,
    $school_crawl_geturi_headers_buf;

  school_crawl_logf($school_crawl_log, 7, "school_crawl_geturi('%s').", $uri);

  $options += array(
    'writefunc' => 'school_crawl_geturi_writefunc_cb',
    'writestate' => '',
  );
  $school_crawl_geturi_writefunc = $options['writefunc'];
  $GLOBALS['school_crawl_geturi_writestate'] = &$options['writestate'];

  $curl = curl_init();
  curl_setopt($curl, CURLOPT_USERAGENT, SP_PACKAGE_NAME . '/' . SP_PACKAGE_VERSION);

  if ($curlsetup_hook !== NULL)
    $curlsetup_hook($curl);

  $school_crawl_geturi_headers_buf = '';
  curl_setopt($curl, CURLOPT_URL, $uri);

  $cookies_str = '';
  foreach ($cookies as $key => $val)
    {
      if (strlen($cookies_str))
	$cookies_str .= ';';
      $cookies_str .= $key . '=' . $val;
    }

  school_crawl_logf($school_crawl_log, 10, "cookies sent: %s", $cookies_str);
  curl_setopt($curl, CURLOPT_COOKIE, $cookies_str);
  curl_setopt($curl, CURLOPT_HEADERFUNCTION, 'school_crawl_geturi_header_cb');
  curl_setopt($curl, CURLOPT_WRITEFUNCTION, 'school_crawl_geturi_write_cb');

  if ($post != NULL && is_array($post))
    {

      /* var_dump($post); */

      $posttxt = '';
      foreach ($post as $postkey => $postvals)
	{
	  /*
	   * This not escaping MEMBER thing is Calvin-specific
	   * too. Maybe we need a way to ask for some particular char
	   * not to be encoded?
	   */

	  /*
	   * Apparently, browsers like seamonkey will send multiple
	   * versions of <input type="hidden" name="field" value="1"
	   * /> if another input exists with name="field", like:
	   * field=1&field=blah. It seems like the webserver for
	   * ccbcmd cares about having these multiple values too...
	   *
	   * Yes, sending subj_sel=dummy&subj_sel=%25 made _all_ of
	   * the difference. Wow.
	   */
	  if (!is_array($postvals))
	    $postvals = array($postvals);
	  foreach ($postvals as $postval)
	    $posttxt .= (strlen($posttxt) ? '&' : '')
	    . urlencode($postkey) . '=' . (strpos($postkey, 'MEMBER') === FALSE ? urlencode($postval) : $postval);
	}
      school_crawl_logf($school_crawl_log, 10, "Setting POST to %s", $posttxt);

      /* curl_setopt($curl, CURLOPT_POST, TRUE); */
      curl_setopt($curl, CURLOPT_POSTFIELDS, $posttxt);
    }

  if (!curl_exec($curl))
    school_crawl_logf($school_crawl_log, 4, "curl_exec() error for uri=%s: %s", $uri, curl_error($curl));
  curl_close($curl);

  $location = NULL;
  foreach (explode("\r\n", $school_crawl_geturi_headers_buf) as $header)
    {
      /*
       * yes, we don't want the line if the first char is a ':' or if it has no ':'
       */
      if (!strpos($header, ':'))
	continue;
      list($header_name, $header_val) = explode(': ', $header, 2);

      school_crawl_logf($school_crawl_log, 9, "%s: %s", $header_name, $header_val);

      switch($header_name)
	{
	case 'Set-Cookie':
	  list($cookie_name, $cookie_val) = explode('=', $header_val, 2);
	  if (isset($cookies[$cookie_name]))
	    school_crawl_logf($school_crawl_log, 10, "Replacing cookie %s=%s with...", $cookie_name, $cookies[$cookie_name]);
	  school_crawl_logf($school_crawl_log, 10, "...new cookie %s=%s.", $cookie_name, $cookie_val);
	  $cookies[$cookie_name] = $cookie_val;
	  break;

	case 'Location':
	  $location = $header_val;
	  /* yes, a calvin-specific replacement :-/ */
	  $location = preg_replace(';(kvdata\.calvin\.edu/)(WebAdvisor);', '\1walive/\2', $location) . "\n";
	  $post = NULL;
	  break;
	}
    }

  if ($follow_meta_refresh)
    {
      $dom = new DOMDocument();
      $dom->loadHTML($options['writestate']);
      foreach ($dom->getElementsByTagName('meta') as $meta_node)
	if ($meta_node->hasAttribute('http-equiv')
	    && !strcasecmp('refresh', $meta_node->getAttribute('http-equiv')))
	  {
	    $meta_content = $meta_node->getAttribute('content');
	    school_crawl_logf($school_crawl_log, 7, "Following http-equiv Refresh: %s", $meta_content);
	    if (!(preg_match('/^[0-9]+; *url=(.*)$/', $meta_content, $meta_matches)))
	      {
		school_crawl_logf($school_crawl_log, 0, "Error following http-equiv Refresh: %s", $meta_content);
	      }
	    else
	      {
		$location = $meta_matches[1];
		$post = NULL;
	      }
	  }
    }

  school_crawl_logf($school_crawl_log, 10, "%s", $options['writestate']);
  if ($location && $loopspin < 6)
    {
      $uri = $location;
      return school_crawl_geturi($uri, $cookies, $school_crawl_log, $post, $follow_meta_refresh, $curlsetup_hook, $options, $loopspin + 1);
    }
  return $options['writestate'];
}

function school_crawl_geturi_header_cb($curl, $header_buf)
{
  global $school_crawl_geturi_headers_buf;
  $school_crawl_geturi_headers_buf .= $header_buf;
  return strlen($header_buf);
}

function school_crawl_geturi_writefunc_cb(&$writebuf, $data)
{
  $writebuf .= $data;
  return strlen($data);
}

function school_crawl_geturi_write_cb($curl, $write_buf)
{
  global $school_crawl_geturi_writefunc, $school_crawl_geturi_writestate;
  return $school_crawl_geturi_writefunc($school_crawl_geturi_writestate, $write_buf);
}

/**
 * \brief
 *   Finds the closest parent of a DOM element with a certain tag
 *   name.
 *
 * Useful for finding the <form /> element associated with a given
 * <select /> or set of <input />s so that the form's action=""
 * parameter may be found.
 *
 * The node itself passed in will be considered for whether or not it
 * matches the $element_name.
 *
 * \param $node
 *   The dom node whose ancestor should be found.
 * \param $element_name
 *   The name of the ancestor element which is requested.
 * \return
 *   The DOMElement sought or NULL if not found.
 */
function school_crawl_element_ancestor(DOMElement $node, $element_name)
{
  if (!strcmp($node->tagName, $element_name))
    return $node;
  if ($node->parentNode)
    return school_crawl_element_ancestor($node->parentNode, $element_name);
  return NULL;
}

/**
 * \brief
 *   Create an array based on an HTML form for submitting the form.
 *
 * Currently, this will only support the <input /> and <select />
 * elements.
 *
 * \param $form_node
 *   The dom node of the form.
 * \return
 *   An array suitable for passing to school_crawl_geturi().
 */
function school_crawl_form(DOMElement $form_node)
{
  $form = array();

  $xpath = new DOMXPath($form_node->ownerDocument);
  foreach ($xpath->query('.//input', $form_node) as $input_node)
    {
      if ($input_node->hasAttribute('name'))
	{
	  /*
	   * Skip over checkboxes which are not ``successful''
	   * (http://w3.org/ terminology).
	   */
	  if (!strcasecmp($input_node->getAttribute('type'), 'checkbox')
	      && !$input_node->hasAttribute('checked'))
	    continue;

	  $input_name = $input_node->getAttribute('name');
	  if (!isset($form[$input_name]))
	    $form[$input_name] = array();
	  if ($input_node->hasAttribute('value'))
	    $form[$input_name][] = $input_node->getAttribute('value');
	  else
	    /* not sure about what best to do in this case... */
	    $form[$input_name][] = '';
	}
    }

  foreach ($xpath->query('.//select', $form_node) as $select_node)
    {
      if ($select_node->hasAttribute('name'))
	{
	  $select_name = $select_node->getAttribute('name');
	  if (!isset($form[$select_name]))
	    $form[$select_name] = array();
	  foreach ($xpath->query('.//option[@selected]', $select_node) as $option_node)
	    if ($option_node->hasAttribute('value'))
	      $form[$select_name][] = $option_node->getAttribute('value');
	}
    }

  return $form;
}

/**
 * \brief
 *   Parses a <select />'s <option />s into an associative array.
 *
 * \param $select
 *   The DOMElement representing the <select />.
 * \param $get_textcontent
 *   Whether or not the returned array should point at DOMElements or
 *   at the textcontent of each <option /> node. Defaults to
 *   textcontent.
 * \param $selected
 *   Will be set to an array of the currently selected keys if passed.
 * \return
 *   An associative array mapping an <option />'s value attribute onto
 *   an array of different <option />s' textcontent (if
 *   $get_textcontent is TRUE) or onto the <option />s' DOMElements
 *   themselves.
 */
function school_crawl_form_select_array(DOMElement $select, $get_textcontent = TRUE, &$selected = NULL)
{
  $selected = array();
  $options = array();
  $name = $select->getAttribute('name');

  foreach ($select->childNodes as $child_node)
    if ($child_node->nodeType == XML_ELEMENT_NODE
	&& !strcasecmp($child_node->tagName, 'option')
	&& $child_node->hasAttribute('value'))
      {
	$value = $child_node->getAttribute('value');
	if ($child_node->hasAttribute('selected'))
	  $selected[] = $value;

	if (empty($options[$value]))
	  $options[$value] = array();
	if ($get_textcontent)
	  $options[$value][] = $child_node->textContent;
	else
	  $options[$value][] = $child_node;
      }

  return $options;
}

/**
 * \brief
 *   Resolve a relativish URL.
 *
 * \param $orig_url
 *   The original URL.
 * \param $url
 *   The new URL to be reconciled with the original one.
 * \return
 *   A string, the new URL.
 */
function school_crawl_url($orig_url, $url)
{
  /*
   * This accounts for both if the $url is already an absolute, fully
   * qualified URL. It falls back to the original URL if it fails to
   * match.
   */
  foreach (array($orig_url, $url) as $aurl)
    if (preg_match(';^(https?)://([^/]+)(/.*)$;', $aurl, $matches))
      {
	if (!empty($matches[1]))
	  $new_url['schema'] = $matches[1];
	if (!empty($matches[2]))
	  $new_url['hostname'] = $matches[2];
	if (!empty($matches[3]))
	  $new_url['path'] = $matches[3];
      }

  /* check if we have an absolute relative path */
  if (!strncmp($url, '/', 1))
    $new_url['path'] = $url;

  /* relative */
  while (!strncmp($url, '../', 3))
    {
      $new_url['path'] = preg_replace(';[^/]+/[^/]+$;', '/', dirname($new_url['path']));
      $url = substr($url, 3);
    }
  if (strncmp($url, '/', 1) && strpos($url, '://') === FALSE)
    $new_url['path'] .= $url;

  return $new_url['schema'] . '://' . $new_url['hostname'] . $new_url['path'];
}

/**
 * \brief
 *   Map a name onto a column of the table with the help of <th />.
 *
 * This should be a quite reliable way of matching the data that a
 * user sees onto the actual data because, in most cases, HTML writers
 * are forced to properly align <th /> and the following hundreds of
 * <td />s for there to be a visual alignment.
 *
 * \param $tr_node
 *   The <tr /> with the <th /> elements to resolve.
 * \param $column_name
 *   The name of the column to search for.
 * \param $strcmp

 *   The function to use with a strcmp($text_content, $column_name) interface when judging
 *   whether or not a <th />'s textContent matches $column_name.

 * \param $trim
 *   The function to apply to the <th />'s textContent before
 *   subjecting it to the $strcmp test.
 * \return
 *   The 0-based index of the column offset or FALSE if the item isn't
 *   found. This index ignores the existence of text elements, so be
 *   careful in using the result.
 */
function school_crawl_table_resolve_column(DOMElement $tr_node, $column_name, $strcmp = 'strcasecmp', $trim = 'trim')
{
  $th_nodelist = school_crawl_table_rownodes($tr_node);
  for ($i = 0; $i < $th_nodelist->length; $i ++)
    if (!$strcmp($trim($th_nodelist->item($i)->textContent), $column_name))
      return $i;
  return FALSE;
}

/**
 * \brief
 *   A strcmp() compatible function for testing regular expressions
 *   for school_crawl_table_resolve_column()'s $strcmp argument.
 */
function school_crawl_table_resolve_column_regexcmp($text_content, $column_name_regex)
{
  if (preg_match($column_name_regex, $text_content))
    return 0;
  return 1;
}

/**
 * \brief
 *   Get a DOMNodeList of a row's elements without #text elements in
 *   the way.
 *
 * Helpful when using school_crawl_table_resolve_column() to get data.
 *
 * \return
 *   A DOMNodeList.
 */
function school_crawl_table_rownodes(DOMElement $tr_node)
{
  $xpath = new DOMXPath($tr_node->ownerDocument);
  return $xpath->query('descendant::*[self::th or self::td]', $tr_node);
}

/**
 * \brief
 *   Resolve an index into a table row, supporting colspan fuzzyish
 *   indexing.
 *
 * \param $tr_node
 *   The row to index into.
 * \param $i
 *   The zero-based index to index as.
 * \return
 *   The DOM Node at that index.
 */
function school_crawl_table_rownode_index(DOMNodeList $rownodes, $i)
{
  foreach ($rownodes as $data_node)
    {
      $colspan = 1;
      if ($data_node->hasAttribute('colspan'))
	$colspan = $data_node->getAttribute('colspan');
      /** \todo check validity of colspan */

      $i -= $colspan;
      if ($i < 0)
	return $data_node;
    }
  return NULL;
}

/**
 * \brief
 *   Detect if a point in a buffer is at a newline.
 *
 * \internal
 *   Used by school_crawl_csv_parse().
 * \param $data
 *   The buffer
 * \param $len
 *   Number of bytes in buffer.
 * \param $i
 *   The location within the buffer.
 * \param $eof
 *   The end of the buffer is the end of the file.
 * \return
 *   The location of the next character after the EOL sequence or
 *   FALSE if there is no EOL.
 */
function _school_crawl_csv_parse_eol($data, $len, $i = 0, $eof = FALSE)
{
  if ($len <= $i)
    return $eof ? $i : FALSE;
  if ($data[$i] == "\n")
    return $i + 1;
  if ($data[$i] == "\r" && $len > $i + 1 && $data[$i + 1] == "\n")
      return $i + 2;
  return FALSE;
}

/**
 * \brief
 *   Read a string of CSV and return it as an array of row arrays.
 *
 * \param $data
 *   CSV data to parse. Parsed data shall be deleted.
 * \param $options
 *   An array with any number of the following optional arguments
 *   which have the documented defaults:
 *   - delimiter (','): The character which delimits fields.
 *   - eof (FALSE): Whether there will be no more data coming.
 *     Normally, if the $data ends without a newline this function
 *     will assume that it cannot assume that there is an implicit
 *     newline. Some improper files don't have the extra newline at
 *     their end and thus this is needed to support them.
 *   - stream (unset): If set to an array containing the keys
 *     'callback' and 'state', will call the 'callback' which is a
 *     function($state, $row) with $state set to the value in 'state'
 *     instead of storing all rows and returning them all.
 * \return
 *   An array with an entry for each line in the CSV file where each
 *   line's entry is an array of the items in that row. An empty array
 *   will be returned in the case that there is insufficient data to
 *   read a line (or insufficient data to tell if the line is

 *   complete, see $options['eof']). If the 'stream' option is set in
 *   $options, then the return value shall be the number of rows
 *   parsed.
 */
function school_crawl_csv_parse(&$data, array $options = array())
{
  $options += array(
    'delimiter' => ',',
    'eof' => FALSE,
  );

  $i = 0;
  $last_line_i = $i;
  $strlen_data = strlen($data);
  $streammode = !empty($options['stream']);
  if ($streammode)
    $ret = 0;
  else
    $ret = array();

  while ($i < $strlen_data)
    {
	$row = array();

	$quote = FALSE;
	$entry = '';
	while ($quote
	       || (_school_crawl_csv_parse_eol($data, $strlen_data, $i, $options['eof']) === FALSE))
	  {
	    /*
	     * There are two ways to read data. One within the
	     * doublequoted entry and the other outside of that.
	     */
	    if ($quote)
	      switch ($data[$i])
		{
		case '"':
		  /*
		   * This is either a lone quote or the terminating
		   * quote. It is a terminating quote if the next
		   * character is EOL or non-quote.
		   */
		  if ($strlen_data > $i + 1 && $data[$i + 1] != '"'
		      || _school_crawl_csv_parse_eol($data, $strlen_data, $i + 1, $options['eof']) !== FALSE)
		    {
		      $quote = FALSE;
		      /*
		       * can't fall-through for this case, eat the
		       * doublequote.
		       */
		      break;
		    }
		  else
		    {
		      /*
		       * We got `""' inside of a doublequoted string,
		       * which is CSV's way of escaping a
		       * doublequote. Thus, eat one of the two
		       * doublequotes.
		       */
		      $i ++;
		      /*
		       * or we don't yet have enough data... The outer
		       * loop also will break on its own in this case...
		       */
		      if ($strlen_data <= $i)
			break;
		      /* fall-through to append doublequote */
		    }

		default:
		  $entry .= $data[$i];
		}
	    else /* if ($quote) */
	      switch ($data[$i])
		{
		case '"':
		  /**
		   * \todo
		   *   Decide if we want to parse ``a,b"c",d'' as
		   *   ["a", "b\"c\"", "d"] or (current) ["a", "bc",
		   *   "d"].
		   */
		  $quote = TRUE;
		  break;

		case $options['delimiter']:
		  $row[] = $entry;
		  $entry = '';
		  break;

		default:
		  $entry .= $data[$i];
		}

	    $i ++;
	    if ($i >= $strlen_data)
	      break;
	  }

	/* Ignore read row because if we encountered end of buffer */
	if (($next_i = _school_crawl_csv_parse_eol($data, $strlen_data, $i, $options['eof'])) === FALSE)
	  break;

	$i = $next_i;
	$last_line_i = $i;
	$row[] = $entry;
	if ($streammode)
	  {
	    $options['stream']['callback']($options['stream']['state'], $row);
	    $ret ++;
	  }
	else
	  $ret[] = $row;
    }

  if (!empty($last_line_i))
    {
      $data = substr($data, $last_line_i);
      if ($data === FALSE)
	$data = '';
    }

  return $ret;
}
