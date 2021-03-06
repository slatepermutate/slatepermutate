<?php /* -*- mode: php; -*- */
/*
 * Copyright 2015 Nathan Phillip Brink <ohnobinki@ohnopublishing.net>
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
   * \todo
   *   See ANT-204 sections 01-09 vs ANT-204-10: section 10 is Honors
   *   students only and even has its title changed to have Honors in
   *   it, others are for normal students.
   */

require_once dirname(dirname(__FILE__)) . DIRECTORY_SEPARATOR . 'inc' . DIRECTORY_SEPARATOR . 'school.crawl.oracle.inc';

function _gvsu_crawl_options(array &$school_crawl_log) {
  return array(
    'curlsetup_hook' => NULL,
    'data_resolver' => new GvsuSchoolCrawlOracleDataResolver($school_crawl_log),
    'html_parser' => new GvsuSchoolCrawlOracleHtmlParser($school_crawl_log),
  );
}

function gvsu_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log) {
  $options = _gvsu_crawl_options($school_crawl_log);
  return school_crawl_oracle_semester_list($school, $semesters, $school_crawl_log, $options);
}

function gvsu_crawl_semester($school, $semester, &$school_crawl_log) {
  $options = _gvsu_crawl_options($school_crawl_log);
  return school_crawl_oracle_crawl_semester($school, $semester, $school_crawl_log, $options);
}

class GvsuSchoolCrawlOracleDataHeadingGetter {
  private $cached;
  private $cached_tr_node;
  private $piece;
  private $school_crawl_log;

  public function __construct(array &$school_crawl_log, $piece) {
    $this->piece = $piece;
    $this->school_crawl_log = &$school_crawl_log;
  }

  function access(DOMNode $tr_node, DOMNodeList $children) {
    if ($this->cached_tr_node === $tr_node) {
      return $this->cached;
    }

    $this->cached = NULL;
    $this->cached_tr_node = $tr_node;

    /*
     * So, basically, <tr>…text that we want</tr><tr>…<table><tr
     * id="$tr_node"/></table></tr>. Thus, we have to go to tr_node’s
     * parent, then seek until we find the closest <tr/> containing
     * that, find its previous sibling, and then get textContent.
     */
    do {
      $tr_node = $tr_node->parentNode;
    } while ($tr_node && $tr_node->nodeName !== 'tr');
    if (!$tr_node)
      /* Apparently this happens sometimes. */
      return $this->cached;

    $tr_node = $tr_node->previousSibling;
    if (!$tr_node) {
      return $this->cached;
    }

    $text = trim($tr_node->textContent);
    /*
     * Example (no newline in original): “Principles of Financial
     * Accounting - 10003 - ACC 212 - 03”
     *
     * But sometimes it is (with newline represented as “\n”): “Linear Algebra III\nLinear Algebra III\nLinear Algebra III\nLinear Algebra III\nLinear Algebra III - 28645 - MTH 406 - 01”
     */
    if (!preg_match('/(?:^|\n|)(.*) - ([[:digit:]]+) - ([^ ]+ [^ ]+) - ([^ ]+)$/', $text, $matches)) {
      throw new Exception('Unable to match course title - course_fqid - section_id with “' . $text . '”');
    }

    $section_id = $matches[3] . ' ' . $matches[4];
    $things = array(
      'title' => $matches[1],
      'registration_number' => $matches[2],
      'section_id' => $section_id,
    );

    /*
     * Then, after the <tr>…text that we want</tr> is a <tr/> that
     * contains a bunch of chunks of text separated, basically, by
     * <br/>. For example:
     */
    if (FALSE) $_ = <<<'EOF'
<TR>
<TH CLASS="ddtitle" scope="colgroup" ><A HREF="/PROD/bwckschd.p_disp_detail_sched?term_in=201610&amp;crn_in=15108">Principles of Financial Accounting - 15108 - ACC 212 - 22</A></TH>
</TR>
<TR>
<TD CLASS="dddefault">
<SPAN class="fieldlabeltext">Associated Term: </SPAN>Fall 2015 
<BR>
<SPAN class="fieldlabeltext">Registration Dates: </SPAN>Mar 16, 2015 to Sep 04, 2015 
<BR>
<SPAN class="fieldlabeltext">Levels: </SPAN>Undergraduate 
<BR>
<BR>
Allendale Campus
<BR>
Lecture and Discussion Schedule Type
<BR>
Traditional Instructional Method
<BR>
       3.000 Credits
<BR>
<A HREF="/PROD/bwckschd.p_disp_syllabus?term_in=201610&amp;crn_in=15108">Textbook Information</A>
EOF;
    /*
     * May also have a preamble with an extra <br/>:
     */
    if (FALSE) $_ = <<<'EOF'
<TD CLASS="dddefault">
Must be an honors student or have a 3.5 GPA.
<BR>
<SPAN class="fieldlabeltext">Associated Term: </SPAN>Fall 2015 
EOF;
    /*
     * Or yet a different set of class="fieldlabeltext" things:
     */
    if (FALSE) $_ = <<<'EOF'
<TD CLASS="dddefault">
<SPAN class="fieldlabeltext">Associated Term: </SPAN>Fall 2015 
<BR>
<SPAN class="fieldlabeltext">Registration Dates: </SPAN>Mar 16, 2015 to Sep 04, 2015 
<BR>
<SPAN class="fieldlabeltext">Levels: </SPAN>Undergraduate 
<BR>
<SPAN class="fieldlabeltext">Attributes: </SPAN>Supplemental Writing Skills 
<BR>
<BR>
Pew Campus
EOF;
    /*
     * So, we’re going to deal with that by assuming that text nodes
     * are normalized and counting <br/>s after the last <span
     * class="fieldlabeltext"/>.
     */
    $tr_node = $tr_node->nextSibling;
    foreach ($tr_node->childNodes as $td_node) {
      if ($td_node->nodeName === 'td') {
        break;
      }
    }
    if ($td_node->nodeName !== 'td') {
      /* Unable to find something, things and stuff. */
      throw new Exception('Unable to find the td in the one place for loading credits and whatnot.');
    }

    /* seek to first <span class="fieldlabeltext"/> */
    $node = NULL;
    foreach ($td_node->childNodes as $potential_node) {
      if ($potential_node->nodeName === 'span' && $potential_node->hasAttribute('class') && $potential_node->getAttribute('class') === 'fieldlabeltext') {
        $node = $potential_node;
      }
    }
    if (!$node) {
      throw new Exception('Unable to find last <span class="fieldlabeltext"/>.');
    }
    $freeform_attributes = array();
    while ($node) {
      // If the current node was a 'br', then we’ve reached the next
      // offset.
      while ($node && $node->nodeName !== 'br') {
        $node = $node->nextSibling;
      }
      // Advance beyond the 'br'.
      $node = $node->nextSibling;
      if ($node) {
        $freeform_attributes[] = trim($node->textContent);
      }
    }
    $things['campus'] = $freeform_attributes[1];

    // Scan for something that looks like credits and use it as credits or throw.
    foreach ($freeform_attributes as $freeform_attribute) {
      if (preg_match('/^([[:digit:].]*) Credits/', $freeform_attribute, $matches)) {
        $things['credits'] = $matches[1];
        break;
      }
    }
    if (empty($things['credits'])) {
      throw new Exception('Unable to find Credits for section “' . $section_id . '” among these freeform attributes: ' . implode(';', $freeform_attributes));
    }

    return $this->cached = $things;
  }

  public function get(DOMNode $tr_node, DOMNodeList $children) {
    $pieces = $this->access($tr_node, $children);
    if (empty($pieces)) {
      return;
    }
    return $pieces[$this->piece];
  }

  public function valid(DOMNode $tr_node, DOMNodeList $children) {
    $pieces = $this->access($tr_node, $children);
    return !empty($pieces[$this->piece]);
  }

  public function __toString() {
    return 'h[' . $this->piece . ']';
  }
}

/**
 * \todo
 *   support resolving and getting credits.
 */
class GvsuSchoolCrawlOracleDataResolver extends SchoolCrawlOracleDataResolver {
  private $has_heading_getter = array(
    'title' => TRUE,
    'registration_number' => TRUE,
    'section_id' => TRUE,
    'credits' => TRUE,
    'campus' => TRUE,
  );

  public function __construct(array &$school_crawl_log) {
    parent::__construct($school_crawl_log);

    $this->key_mapping = array(
      'dates' => 'date range',
      'times' => 'time',
      'instructor' => 'instructors',
      'location' => 'where',
    ) + $this->key_mapping;
  }

  public function resolve($key, DOMElement $tr_header_node) {
    if (!empty($this->has_heading_getter[$key])) {
      return new GvsuSchoolCrawlOracleDataHeadingGetter($this->school_crawl_log, $key);
    }

    return parent::resolve($key, $tr_header_node);
  }
}

class GvsuSchoolCrawlOracleHtmlParser extends SchoolCrawlOracleHtmlParser {
  public function __construct(array &$school_crawl_log) {
    parent::__construct($school_crawl_log);
  }

  public function parse($html) {
    // It was reported on 2019-03-19 that the import was not
    // working. This was due to the id="term_input_id" not being read
    // into the DOM correctly because of invalid HTML. The invalid
    // example is as follows:
    //
    //     <select name="p_term" size="1"  BYPASS_ESC=>"Y" ID="term_input_id">
    //
    // Notice the premature “>”.
    //
    // To repair, we do a replacement. It is somewhat specific to try
    // to avoid accidentally munging other data. But, due to the
    // nature of the issue, it’s kind of impossible to guarantee
    // correctness (because we are trying to fix invalid/bad syntax).
    $html = preg_replace('/BYPASS_ESC=>/', 'BYPASS_ESC=', $html);

    return parent::parse($html);
  }
}
