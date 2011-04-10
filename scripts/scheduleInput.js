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

    //--------------------------------------------------
    // General Notes
    //--------------------------------------------------

var classNum = 0;

/**
 * \brief
 *   The number of section entries for a given course.
 *
 * Key is course_i, value is the current number of sections.
 */
var sectionsOfClass = new Array();

/**
 * \brief
 *   Help to generate a unique section identifier for each section
 *   added to a given course.
 *
 * Necessary to support PHP-style post array thingies, like
 * classes[0][1][$x] would be all of the data for course_i=0,
 * section_i=1, variable $x (ex. day of week, start time, end time,
 * teacher). We can't have two sections for a given course using the
 * same section_i because those values would override eachother.
 */
var last_section_i = 0;

/**
 * \brief
 *   A quick hash to prevent there from being two AJAX requests for a
 *   given course at one time.
 */
var course_ajax_requests = [];

/**
 * \brief
 *   The course number which contains nothing.
 *
 * To avoid having a user need to click the ``Add course'' button, we
 * keep a course added at the end of the list of courses. If this
 * variable is -1, it indicates that no such free course exists. If it
 * is zero or greater, that number is the class which is the free one.
 */
var slate_permutate_course_free = -1;

/*
 * General Input Functions
 */

/**
 * Outputs an <option/> element. It will inlcude selected="selected"
 * if the value param equals the test_value param.
 */
function genOptionHtml(value, content, test_value)
{
    var selected = ' selected="selected"';
    if (value != test_value)
	selected = '';
    return '<option value="' + value + '"' + selected + '>' + content + "</option>\n";
}


/** Add tooltips for user guidance */
function addTips()
{
  var tr = jQuery('tr');
  var td = tr.eq(tr.length-1);

 /* slate_permutate_example_course_id is set globally in input.php. */
 jQuery('td:first', td).qtip(
   {
      content: 'Start typing your class ID (such as ' + slate_permutate_example_course_id + ') and click a suggestion to add sections',
      style: {
        tip: true,
        classes: "ui-tooltip-dark ui-tooltip-shadow ui-tooltip-rounded"
      },
      show: {
        ready: true
      },
      position:{
        my: 'top left', 
        at: 'bottom right'
      }
    }
  );

}

/**
 * \brief
 *   Add a section to a class.
 */
function add_section_n(cnum, name, synonym, stime, etime, days, instructor, location, type)
{
    var snum = last_section_i ++;
    var cssclasses = 'section class' + cnum;

    if(type == 'lab')
	cssclasses += ' lab';

    var section_html = '<tr id="tr-section-' + String(snum) + '" class="' + cssclasses + '"><td class="none"></td>' +
	'<td class="sectionIdentifier center"><input type="text" size="1" class="required section-letter-entry" name="postData[' + cnum + '][' + snum + '][letter]" /><input class="section-synonym-entry" type="hidden" name="postData[' + cnum + '][' + snum + '][synonym]" /></td>' +
	'<td class="professor center"><input type="text" size="10" class="profName" name="postData[' + cnum + ']['+ snum + '][professor]" /></td>' +
	'<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][start]"><option value="none"></option>' +
	genOptionHtml("0700", "7:00 am", stime) + genOptionHtml("0730", "7:30 am", stime) +
	genOptionHtml("0800", "8:00 am", stime) + genOptionHtml("0830", "8:30 am", stime) +
	genOptionHtml("0900", "9:00 am", stime) + genOptionHtml("0930", "9:30 am", stime) +
	genOptionHtml("1000", "10:00 am", stime) + genOptionHtml("1030", "10:30 am", stime) +
	genOptionHtml("1100", "11:00 am", stime) + genOptionHtml("1130", "11:30 am", stime) +
	genOptionHtml("1200", "12:00 pm", stime) + genOptionHtml("1230", "12:30 pm", stime) +
	genOptionHtml("1300", "1:00 pm", stime) + genOptionHtml("1330", "1:30 pm", stime) +
	genOptionHtml("1400", "2:00 pm", stime) + genOptionHtml("1430", "2:30 pm", stime) +
	genOptionHtml("1500", "3:00 pm", stime) + genOptionHtml("1530", "3:30 pm", stime) +
	genOptionHtml("1600", "4:00 pm", stime) + genOptionHtml("1630", "4:30 pm", stime) +
	genOptionHtml("1700", "5:00 pm", stime) + genOptionHtml("1730", "5:30 pm", stime) +
	genOptionHtml("1800", "6:00 pm", stime) + genOptionHtml("1830", "6:30 pm", stime) +
	genOptionHtml("1900", "7:00 pm", stime) + genOptionHtml("1930", "7:30 pm", stime) +
	genOptionHtml("2000", "8:00 pm", stime) + genOptionHtml("2030", "8:30 pm", stime) +
	genOptionHtml("2100", "9:00 pm", stime);

    if (stime.length > 0)
    {
	var stime_end = stime.substr(2);
	var stime_begin = stime.substr(0, 2);
	if (stime_end != '00' && stime_end != '30'
	   || stime_begin < 7 || stime_begin > 21)
	    section_html = section_html + genOptionHtml(stime, prettyTime(stime), stime);
    }

    section_html = section_html + '</select></td>\
<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][end]"><option value="none"></option>' +
	genOptionHtml("0720", "7:20 am", etime) + genOptionHtml("0750", "7:50 am", etime) +
	genOptionHtml("0820", "8:20 am", etime) + genOptionHtml("0850", "8:50 am", etime) +
	genOptionHtml("0920", "9:20 am", etime) + genOptionHtml("0950", "9:50 am", etime) +
	genOptionHtml("1020", "10:20 am", etime) + genOptionHtml("1050", "10:50 am", etime) +
	genOptionHtml("1120", "11:20 am", etime) + genOptionHtml("1150", "11:50 am", etime) +
	genOptionHtml("1220", "12:20 pm", etime) + genOptionHtml("1250", "12:50 pm", etime) +
	genOptionHtml("1320", "1:20 pm", etime) + genOptionHtml("1350", "1:50 pm", etime) +
	genOptionHtml("1420", "2:20 pm", etime) + genOptionHtml("1450", "2:50 pm", etime) +
	genOptionHtml("1520", "3:20 pm", etime) + genOptionHtml("1550", "3:50 pm", etime) +
	genOptionHtml("1620", "4:20 pm", etime) + genOptionHtml("1650", "4:50 pm", etime) +
	genOptionHtml("1720", "5:20 pm", etime) + genOptionHtml("1750", "5:50 pm", etime) +
	genOptionHtml("1820", "6:20 pm", etime) + genOptionHtml("1850", "6:50 pm", etime) +
	genOptionHtml("1920", "7:20 pm", etime) + genOptionHtml("1950", "7:50 pm", etime) +
	genOptionHtml("2020", "8:20 pm", etime) + genOptionHtml("2050", "8:50 pm", etime) +
	genOptionHtml("2120", "9:20 pm", etime) + genOptionHtml('2150', '9:50 pm', etime);

    if (etime.length > 0)
    {
	var etime_end = etime.substr(2);
	var etime_begin = etime.substr(0, 2);
	if (etime_end != '50' && etime_end != '20'
	   || etime_begin < 7 || etime_begin > 21)
	    section_html = section_html + genOptionHtml(etime, prettyTime(etime), etime);
    }

    section_html = section_html + '</select></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][0]" value="1" ' + (days.m ? 'checked="checked"' : '') + ' /></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][1]" value="1" ' + (days.t ? 'checked="checked"' : '') + ' /></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][2]" value="1" ' + (days.w ? 'checked="checked"' : '') + ' /></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][3]" value="1" ' + (days.h ? 'checked="checked"' : '') + ' /></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][4]" value="1" ' + (days.f ? 'checked="checked"' : '') + ' /></td>\
<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][5]" value="1" ' + (days.s ? 'checked="checked"' : '') + ' /></td>' +
	'<td class="removeCell"><div class="deleteSection"><input type="button" value="x" class="gray" /></div></td><td class="emptyCell">' +
	'<input class="section-location-entry" type="hidden" name="postData[' + cnum + '][' + snum + '][location]" />' +
	'<input class="section-type-entry" type="hidden" name="postData[' + cnum + '][' + snum + '][type]" />' +
	'</td></tr>';

    jQuery('tr.class' + cnum + ':last').after(section_html);
    sectionsOfClass[cnum] ++;

    var section_tr = jQuery('#tr-section-' + String(snum));
    /* store course_i in a place the newly added section will look for it */
    section_tr.data({course_i: cnum});

    /*
     * Store data into the newly created HTML. With this method we
     * have to _avoid_ escaping entities in the text we're setting as
     * values because the DOM stuff will escape it for us.
     */
    section_tr.find('.section-letter-entry').val(name);
    section_tr.find('.section-synonym-entry').val(synonym);
    section_tr.find('.profName').val(instructor);
    section_tr.find('.section-location-entry').val(location);
    section_tr.find('.section-type-entry').val(type);

    /* unhide the saturday columns if it's used by autocomplete data */
    if (days.s)
	jQuery('#jsrows col.saturday').removeClass('collapsed');

    return last_section_i - 1;
}
function add_section(cnum)
{
    var section_i = add_section_n(cnum, '', '', '', '', {m: false, t: false, w: false, h: false, f: false, s: false}, '', '', '');
    if (cnum == slate_permutate_course_free)
	course_free_check(cnum);
    return section_i;
}

/**
 * Add a list of sections gotten via an AJAX call.
 */
function add_sections(cnum, data)
{
    var i;

    if (data.title)
	jQuery('.pclass' + cnum + ' .course-title-entry').val(data.title);

    /*
     * If the user enterred something iffy, correct him. Or do so
     * regardless ;-).
     */
    /* this data['class'] stuff is for the old JSON format we used... */
    if (data['class'])
	data.course = data['class'];
    if (data.course)
	jQuery('.className' + cnum).val(data.course);

    if (!data.sections)
	return;

    jQuery.each(data.sections, function(i, section)
		{
		    add_section_n(cnum, section.section, section.synonym, section.time_start, section.time_end, section.days, section.instructor, section.location, section.type);
		});

    /*
     * Handle course-level interdependencies.
     */
    if (data.dependencies)
	jQuery.each(data.dependencies, function(i, dep)
		    {
			/* Gracefully deprecate the old crawler's JSON format. */
			if (dep['class'])
			    dep.course = dep['class'];

			var new_course_num = add_class_n(dep.course, dep['title'] ? dep['title'] : '');
			add_sections(new_course_num, dep);
		    });
}

/**
 * \brief
 *   Adds a new class to the input.
 *
 * \param course_id
 *   The course_id.
 * \param title
 *   The human-friendly course title.
 * \return
 *   The javascript-local course entry identifying number.
 */
function add_class_n(course_id, title)
{
    /*
     * If we're adding a course entry form with preadded
     * content, first remove the empty course.
     */
    if (course_id.length && slate_permutate_course_free != -1)
	course_remove(slate_permutate_course_free);

    sectionsOfClass[classNum] = 0; // Initialize at 0
    course_ajax_requests[classNum] = false;
    jQuery('#jsrows').append('<tr id="tr-course-' + classNum + '" class="class class' + classNum + ' pclass' + classNum + '"><td class="nameTip"><input type="text" id="input-course-' + classNum + '" class="classRequired defText className'+classNum+' className" title="Class Name" name="postData[' + classNum + '][name]" /></td><td colspan="10"><input type="text" name="postData[' + classNum + '][title]" class="inPlace course-title-entry" /></td><td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" class="gray" /></div></td><td class="none"><button type="button" class="addSection gray">+</button></td></tr>');

		/* store classNum as course_i into the <tr />: */
    var tr_course = jQuery('#tr-course-' + classNum);
    tr_course.data({course_i: classNum});
    tr_course.find('.course-title-entry').val(title);
    tr_course.find('.className').val(course_id);

		var class_elem = jQuery('.className' + classNum);

		class_elem.autocomplete({ source: 'auto.php?school=' + slate_permutate_school + '&semester=' + slate_permutate_semester });
		class_elem.bind('autocompleteselect', {class_num: classNum, class_elem: class_elem},
			function(event, ui)
			    {
				if (!ui.item)
				    return;

				if (ui.item.value.indexOf('-') != -1)
				    {
					course_autocomplete(event.data.class_num, ui.item.value);
				    }
				else
				    {
					/*
					 * The user selected a department, such as CS or MATH.
					 * Thus, we should append a '-' to the value and do a search for that.
					 */
					var newval = ui.item.value + '-';
					event.data.class_elem.
					    val(newval).
					    autocomplete("search", newval);

					/* void out the default event since we are setting the value ourselves, with a '-' */
					event.preventDefault();
				    }
			    });

		classNum++;

		return (classNum - 1);
	}

/**
 * \brief
 *   Ensure that there is an empty course entry and return its
 *   identifier.
 */
function add_class()
{
    /*
     * Don't add an empty new course entry if there already is
     * one. Otherwise, set this new class to be the ``hot'' one.
     */
    if (slate_permutate_course_free == -1)
	slate_permutate_course_free = add_class_n('', '');
    return slate_permutate_course_free;
}

/**
 * \brief
 *   Try to fetch a section once the user has chosen an autocomplete
 *   entry.
 *
 * Since this can be called also when the user just types in a course
 * and hits enter without what he typed necessarily matching an
 * autocomplete item, this function handles the case where the
 * requested course might not have information on the server.
 *
 * \param course_i
 *   The javascript/postData index of the course to autocomplete.
 * \param term
 *   The term which the user entered. Optional.
 * \return
 *   Nothing.
 */
function course_autocomplete(course_i, term)
{
    var course_name_elem = jQuery('.className' + course_i);

    /*
     * A safety mechanism: don't autocomplete a course if it already
     * has sections. Since this is AJAX, this same check must also
     * show up in the AJAX callback.


     */
    if (course_ajax_requests[course_i] || sectionsOfClass[course_i])
	return;

    course_ajax_requests[course_i] = true;

    if (jQuery.type(term) == 'undefined')
	term = course_name_elem.val();

    jQuery.ajax(
	{
	    url: 'auto.php',
	    complete: function()
	    {
		/*
		 * Not matter how the request goes -- if it fails or
		 * returns nothing or whatnot -- the channel must be
		 * opened up for more AJAX requests.
		 */
		course_ajax_requests[course_i] = false;
	    },
	    data: {
    		getsections: 1,
		term: term,
		school: slate_permutate_school,
		semester: slate_permutate_semester
	    },
	    success: function(data, textStatus, reqobj)
	    {
		var new_course_num;

		if (data.sections)
		{
		    if (sectionsOfClass[course_i])
			return;

		    add_sections(course_i, data);
		    /* Close the autocomplete menu thingy. */
		    course_name_elem.autocomplete('close');

		    new_course_num = add_class();

		    if (course_name_elem.val() != data.course_id)

		    /* position the user's cursor the new class's input box */
		    jQuery('#input-course-' + new_course_num).focus();
		}
	    }
	}
    );

    return;
}

/**
 * \brief
 *   Remove a course entry.
 *
 * Ensures that slate_permutate_course_free is kept consistent.
 *
 * \param course_i
 *   The internal JS identifer for the course (not the course_id which
 *   the PHP cares about).
 */
function course_remove(course_i)
{
    jQuery('.class' + course_i).remove();

    /*
     * Check if the class intended for the user to
     * enter information into has been removed.
     */
    if (slate_permutate_course_free == course_i)
	slate_permutate_course_free = -1;
}

/**
 * \brief
 *   Figure whether or not a given course entry has sections.
 *
 * \param course_i
 *   The internal javascript representation of a course entry.
 * \return
 *   true or false.
 */
function course_has_sections(course_i)
{
    return sectionsOfClass[course_i] > 0;
}

/**
 * \brief
 *   Figure out whether or not an empty course entry has become filled
 *   or whether a full course has become emptied and react.
 *
 * This mainly ensures that there is always exactly one course entry
 * spot, eliminating the need of an ``Add class'' button.
 *
 * \param course_i
 *   If this is not being called as a 'change' or 'keyup' event
 *   handler for a <input class="className"/>, then course_i may refer to
 *   a the course_i to check.
 */
function course_free_check(course_i)
{
    var me;
    if (jQuery.type(course_i) == 'number')
	me = jQuery('.pclass' + course_i + ' .className');
    else
	me = jQuery(this);

    course_i = me.parent().parent().data('course_i');
    if (course_i == slate_permutate_course_free && (me.val().length || course_has_sections(course_i)))
	{
	    /* I am no longer the empty course entry */
	    slate_permutate_course_free = -1;
	    add_class();
	}
    if (course_i != slate_permutate_course_free && !(me.val().length || course_has_sections(course_i)))
	{
	    /* I am now an empty course entry */
	    /* kill an other empty course entry if it exists... */
	    if (slate_permutate_course_free != -1)
		course_remove(slate_permutate_course_free);
	    slate_permutate_course_free = course_i;
	}
}

/**
 * \brief
 *   A function to prevent accidental form submission.
 *
 * To be bound to keyup and keydown events for objects which may
 * accidentally be used to cause form submission.
 */
function slate_permutate_nullify_enter(e)
{
    /* <ENTER> is 13 */
    if (e.which == 13)
    {
	/*
	 * The user has pressed enter before selecting an autocomplete entry, which means the
	 * form will be submitted without his expecting it to be. We yet need code
	 * to figure out what the first autocomplete result is :-/.
	 */
	return false;
    }
    return true;
}

/**
 * \brief
 *   Render a slate_permutate-encoded time-of-day.
 *
 * \param time_str
 *   A four-character representation of a time of day based on a
 *   time's 24-hour representation.
 * \return
 *   A string representing the specified time.
 */
function prettyTime(time_str)
{
    var i_hour;
    var hour_str;
    var m;

    i_hour = time_str.substr(0, 2) * 1;
    if (i_hour < 12)
	{
	    m = 'a';
	}
    else
	{
	    m = 'p';
	    if (i_hour > 12)
		i_hour -= 12;
	}
    hour_str = new String(i_hour);
    /* uncomment to have 08:01 instead of 8:01 */
    /*
    while (hour_str.length < 2)
	hour_str = '0' + hour_str;
    */

    return hour_str + ':' + time_str.substr(2) + ' ' + m + 'm';
}

//--------------------------------------------------
// Items bound to pageload/events
//--------------------------------------------------
jQuery(document).ready(function() {

	//--------------------------------------------------
	// Deletes the selected class from input
	//--------------------------------------------------
	jQuery('.deleteClass').live('click', function() {
	    /* The user is not allowed to interactively delete the one empty course */
	    var course_i = jQuery(this).parent().parent().data('course_i');
	    if (slate_permutate_course_free == course_i)
		return false;
	    if(confirm('Delete class and all sections of this class?')) {
		/* The one empty course may have became this course in that time */
		if (slate_permutate_course_free == course_i)
		    return false;
		course_remove(course_i);
		return false;
	    }
	    return false;
	});

	//--------------------------------------------------
	// Deletes the selected section from the input
	//--------------------------------------------------
	jQuery('.deleteSection').live('click', function() {
	  // Decreases the total number of classes
		var course_i = jQuery(this).parent().parent().data('course_i');
		sectionsOfClass[course_i]--;

	  // Find the ID cell of the row we're in
	  var row = jQuery(this).parent().parent().find(".sectionIdentifier");

	  // The first input is the one containing the section ID
	  var toMatch = jQuery(row).find("input").val();
	    
	  // This gets the second class of the row, "class#"
	  var classClass = "." + jQuery(row).parent().attr("class").split(" ")[1];

	  // Iterate over each section of this class
	  jQuery(classClass).each( function() {
	    // If this section has the same course ID as the item clicked, remove it.
	    if(jQuery(this).find("input").val() == toMatch){
		jQuery(this).remove();
	    }
	  });
	  course_free_check(course_i);
	});

	jQuery('.className').live('change', course_free_check).live('keyup', course_free_check);

	//--------------------------------------------------
	// Bind the section-adding method
	//--------------------------------------------------
	jQuery('.addSection').live('click', function() {
		var course_i = jQuery(this).parent().parent().data('course_i');
		add_section(course_i);
	});

	//--------------------------------------------------
	// Default text
	//--------------------------------------------------
	jQuery(".defText").focus(function(srcc)
	{
	    if (jQuery(this).val() == jQuery(this)[0].title)
	    {
		jQuery(this).removeClass("defaultTextActive");
		jQuery(this).val("");
	    }
	});
	jQuery(".defText").blur(function()
	{
	    if (jQuery(this).val().length === 0)
	    {
		jQuery(this).addClass("defaultTextActive");
		jQuery(this).val($(this)[0].title);
	    }
	});
	jQuery(".defText").blur();

	//--------------------------------------------------
	// Show/Hide advanced items
	//--------------------------------------------------
	jQuery('.advanced').hide();    
	jQuery('#showadvanced').click( function() {
		jQuery('#showadvanced').hide();
		jQuery('.advanced').slideToggle();
	});

        //--------------------------------------------------
        // Show/Hide instructions
        //--------------------------------------------------
	jQuery('#schoolInstructionsBox').hide();
	jQuery('#showInstructions').click( function() {
		jQuery('#showInstructions').hide();
		jQuery('#schoolInstructionsBox').slideToggle();
	});


	//-------------------------------------------------
	// Show more saved schedules
	//-------------------------------------------------
        jQuery('#showMore').click( function() {
		jQuery('.hidden').show();
		jQuery('#showMore').hide();
		jQuery('#showLess').show();
        });
        jQuery('#showLess').click( function() {
		jQuery('.hidden').hide();
		jQuery('#showMore').show();
		jQuery('#showLess').hide();
	});


        //-------------------------------------------------
        // Style course titles as inputs when clicked
        //-------------------------------------------------
        jQuery('.course-title-entry').live('click', function() {
          jQuery(this).toggleClass('inPlace');
        });
    /*
     * Prevent accidental form submission for className and course
     * title entry text fields.
     */
    jQuery('.course-title-entry').live('keyup keydown', slate_permutate_nullify_enter);
    jQuery('.className').live('keyup keydown', function(e)
			      {
				  if (e.which == 13)
				  {
				      course_autocomplete(jQuery(this).parent().parent().data('course_i'));

				      /* Prevent form submission like slate_permutate_nullify_enter() does. */
				      return false;
				  }
			      });
        jQuery('.course-title-entry').live('blur', function() {
          jQuery(this).addClass('inPlace');
        });
});
