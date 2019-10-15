#!/usr/bin/env php-cgi
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

  include_once 'inc/class.page.php'; 
$projectpage = page::page_create('About');
$projectpage->head();
?>

<p>
This website was created by <a href="https://calvin.edu" rel="external">Calvin College (now known as Calvin University)</a> and <a href="https://cedarville.edu/" rel="external">Cedarville University</a> students.
  They are now all alumni, but still have connections to college students and gladly maintain SlatePermutate.
</p>

<h3>Contact</h3>
<p>
  Please contact the developers/maintainers using the <a href="feedback.cgi">Feedback</a> for general questions and to report problems.
  If you are interested in development, please visit the <a href="https://bitbucket.org/slatepermutate/slatepermutate">project page on Bitbucket</a>.
</p>

<h3>Stack</h3>
<p>
</p>
<p>
  SlatePermutate is written in PHP, with a valid XHTML frontend.
  It is currently deployed using CGI-style FastCGI (thus the PHP interpreter is launched via a shebang and mod_fcgid instead of mod_php—that is why you see <code>.cgi</code> instead of <code>.php</code> in the URIs these days).
  However, there is interest in moving to something like JavaScript or Haxe (mostly for the static typing) and enabling more of the functionality to be implemented in the client.
  Feel free to dig into the source yourself <a href="https://bitbucket.org/slatepermutate/slatepermutate">at Bitbucket</a>!
</p>

<?php
$projectpage->foot();
