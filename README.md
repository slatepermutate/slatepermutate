slate_permutate: the semester scheduling assistant.

Authors:
  See AUTHORS

Resources:
  Homepage: http://ohnopub.net/w/SlatePermutate
  Support: irc://irc.ohnopub.net/slatepermutate
  Mercurial repo: http://protofusion.org/hg/SlatePermutate/
  Bugzilla: http://protofusion.org/bugzilla/

  Please use the means listed above to give us any feedback about
  slate_permutate's functionality. Thanks!

Installation Requirements:
  - PHP-enabled httpd (tested with apache, patches for supporting others welcome).
  - Access to PHP's CLI interface
  - libcURL extension to PHP (for Calvin's crawler and soon others).
  - json_encode()/json_decode() functions (as a PEAR addon or with the recent PHP versions).
  - PHP libraries
    - securimage-2.0.2 (optional) for captcha support for the feedback
      form. http://phpcapatcha.org/ , see inc/config.example for
      details.

Installation:
  - Get a copy of slate_permutate.
    - development version:
      $ hg clone http://protofusion.org/hg/SlatePermutate slate_permutate
    - stable: not yet available.
  - copy inc/config.inc.example to inc/config.inc. Read and adjust
    settings as necessary.
  - copy .htaccess.example to .htaccess if you're interested in
    ``clean urls'' (corresponding to the $clean_urls option in
    config.inc).
  - run admin/rehash.php to populate the cache/ directory with
    information such as the list and rDNS information for schools in
    school.d.
  - ensure that the webserver has write-access to the saved_schedules/
    folder because schedule storage is filesystem-based.

License:
  - The Affero General Public License version 3. This, in essence, is
    the same as a normal GPL. However, if slate_permutate is use as
    part of a publically-accessible web service and alterations have
    been made, those alterations must be published and available under
    the same license as existing code. However, the above simple
    understanding is no replacement for the actual content of the
    license itself; see COPYING for the license.
