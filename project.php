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

  include_once 'inc/class.page.php'; 
  $projectpage = new page('Project');
?>

<h3>The SlatePermutate Project</h3>

<p>Welcome to the SlatePermutate Project, by Nathan Gelderloos, Ethan Zonca, and Nathan Brink.</p>

<h3>Licensing</h3>
<p>SlatePermutate is licensed under the terms of the GNU Affero General Public License, version 3. SlatePermutate is distributed without any warranty. See the <a href="http://www.gnu.org/licenses/" rel="external">GNU Affero General Public License</a> for more details.</p>

<h3>Bug/Issue Tracking</h3>
<p>To view or file bugs for SlatePermutate, visit the <a href="http://protofusion.org/bugzilla/buglist.cgi?product=SlatePermutate" rel="external">Bugzilla tracker</a> for the project. All issues will be addressed as soon as is reasonably possible.</p>

<h3>Source Code</h3>
<p>A beta version of SlatePermutate will be released in the near future. For now, an alpha snapshot is available for download: <br /><a href="http://mirror.calvin.edu/~binki/slate_permutate-0.1_pre20101110.tar.bz2">slate_permutate-0.1_pre20101110.tar.bz2</a></p>
<p>Current development code can be retrieved via the Mercurial SCM at <em>https://protofusion.org/hg/SlatePermutate</em></p>

<h3>Requirements</h3>
<p>To run properly, SlatePermutate requires a modern PHP environment (5.0+) that includes libcurl, and a writable cache directory for school and schedule data. SlatePermutate <em>does not</em> require a backend database.</p><p>SlatePermutate has optional requirements of Google Analytics tracking and reCaptcha for feedback forms; these options are set in the config.inc file.</p>

<h3>About</h3>
<p>This program was created by <a href="http://www.calvin.edu" rel="external">Calvin College</a> and <a href="http://cedarville.edu/" rel="external">Cedarville University</a> students. SlatePermutate is written in PHP, with a valid XHTML frontend.</p>

<h3>Contact</h3>
<p>You may contact the developers via the <a href="feedback.php">feedback form</a> (preferred), or you can email individual developers as listed in the <a href="http://protofusion.org/hg/SlatePermutate">repository</a>.</p>

<?php
$projectpage->foot();
