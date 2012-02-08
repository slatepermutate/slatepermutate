slate_permutate School Support

slate_permutate would be worthless if it did not support scraping
course scheduling information from school websites. Also, to properly
support a school slate_permutate needs more than just the list of
section meetings from that school. It needs a way to identify the
school and some basic metadata for the school, such as a
human-friendly school identifier, a way to address an individual from
that school, and instructions specific to that school.

In slate_permutate, a school_id is used to identify a school at the
software level. Each school_id is a string such that
http://<school_id>.edu/ accesses the school's website. (One
consideration, maybe it should just be http://<school_id>/ to support
international schools?) For example, the school Calvin College has a
website of http://calvin.edu/ and would thus have a school_id of
calvin.

To add support for a school to slate_permutate, simply create a file
called school.d/<school_id>.inc. Then define at least a PHP function
in that file called <school_id>_info() which conforms to the API
defined below. slate_permutate will see the new school and add it to
its list of schools upon running the admin/rehash.php script.

API:

There are two possible PHP include files for a given school. The
first, necessary file is called <school_id>.inc. It is this file's
existence which clues slate_permutate into that school's
existence. This file is included by slate_permutate each time a user
from that school sends a request to slate_permutate. A second file,
only used when admin/rehash.php is called and only necessary if that
school wants to provide autocomplete support, is called
<school_id>.crawl.inc. The functions which must and may be defined in
each of these files are listed below:

* <school_id>.inc (required)

** <school_id>_info() (required)
   Returns an array with the following keys:
   - 'name' (required): A user-friendly name for the college.
   - 'student_address' (optional): Normally the college's mascot. At
       Calvin College, each student is a ``Knight'' as we are Knights.
   - 'domains' (optional): An array of domain names which the reverse
       DNS lookups of students from this college will fall under. For
       example, a computer connecting from calvin college (but from
       outside of resnet, the dorm network) has a reverse DNS of
       something like dhcp60-3.calvin.edu which falls under
       'calvin.edu'. A 'domains' setting of array('calvin.edu') would
       catch this. The default is array('<school_id>.edu').
   - 'url' (optional): The URL of the school's website if it does not
       fit into the <school_id>.edu scheme.
   - 'registration_url' (optional): A URL to which students interested
       in registering for courses are pointed. This is only effective
       if <shcool_id>_registration_html() is not overridden.

** <school_id>_instructions_html() (optional)

   Returns a string containing a valid XHTML fragment suitable for
   insertion as either sectioning content or flow content. This
   fragment should contain school-specific instructions and is
   displayed currently at the bottom of the input.php page. There is
   default blurb text pulled from the special school with
   school_id=default.

** <school_id>_registration_html(Page $page, array $school, Semester $semester, array $courses) (optional)
   - $page: The Page object (see inc/class.page.php) used to serve the
     request.
   - $school: The school's information array.
   - $semester: A vacuous Semester object for conveying the season,
     year, and semester_id for which the user is intending to
     register.
   - $courses: An array of Course objects containing only the Section
     objects which the user has selected.

   Returns a string containing a valid XHTML fragment suitable for
   insertion as either sectioning content or flow content. This
   fragment is to provide instructions specific to registering for the
   sections the user has selected. If a school has a nice HTTP GET
   registration API, you might be able to provide a <form /> letting
   the user seamlessly request registration of these certain classes.

** <school_id>_default_courses() (optional)

   Returns an array of Course objects which shall automatically be
   placed into a newly-created ``empty'' schedule. The idea, as used
   for Cedarville (school_id=cedarville), is that certain schools
   might enforce certain classes upon students unconditionally (such
   as a daily morning chapel at Cedarville). It is nice to be able to
   see such required meetings on a created schedule. However, in the
   case of Cedarville there is only one meeting time option for
   chapel. Thus, Cedarville's course schedule cannot be expected to
   include any courses which meet during the time that this chapel
   course meets -- therefore the presence of chapel on the schedule
   provides no benefit. But if there were multiple meeting times
   options for a universally required course, such as if there were
   two chapel options at Cedarville, then there would be more use in
   listing the course as a default...

** <school_id>_page_css() (optional)
   Return a string of CSS to insert into every slate_permutate page
   when the user has selected this school.

* <school_id>.crawl.inc (optional)

  It is very strongly recommended that your school.d entry provide a
  crawling interface. This is necessary for the course_id
  autocompletion functionality. It may be extended to be used for
  other purposes later too.

  When writing a crawler interface, it is essential that a
  publicly-accessible data resource be used. Anyone should be able to
  get a copy of slate_permutate and use it to generate autocompletion
  data for your school. You may not use resources or backends which
  are password-restricted since that gives you a monopoly on providing
  slate_permutate for your particular school. Of course, if you want
  to do such a thing you may, but you must publish the source-code
  used to do so (as this project is protected by the Affero GPL) (but
  that doesn't mean you need to publish your password). Just, such a
  school backend module will never be accepted into mainline
  slate_permutate since only those with the proper access privileges
  can maintain such a backend, etc.

  The current crawler API has two steps. First,
  <school_id>_crawl_semester_list() is called to retrieve a list of
  crawlable semesters for the school. Then
  <school_id>_crawl_semester_get() is called once for each semester.

** <school_id>_crawl_semester_list(array $school, array &$semesters, &$school_crawl_log)
  - $school: The school handle for <school_id>.
  - $semesters: An array to which Semester objects should be appended.
  - $school_crawl_log: A reference to the school_crawl_log handle used
    by some school.crawl.inc functions. To log warnings and errors,
    use school_crawl_logf().
  Returns 0 on success and nonzero on failure.

  This function should scrape the school's website and build a list of
  scrapable semesters. For each semester, it should instantiate a
  Semester object with metadata about that semester and then return an
  array of these Semester objects.

** <school_id>_crawl_semester(array $school, Semester $semester, &$school_crawl_log)
  - $school: The school handle for <school_id>
  - $semester: The semester object to which courses are added.
  - $school_crawl_log: The school_crawl_log handle required by some
    school.crawl.inc functions.
  Returns 0 on success and otherwise on failure.

  This function is to add courses (or course_slots) to the passed
  Semester object so that every available course for the given
  semester is catalogued by the Semester object. Upon successful
  return of this function (indicated by returning 0), the caller will
  store the data to disk for use by the autocomplete engine.
