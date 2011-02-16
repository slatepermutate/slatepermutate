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
var sectionsOfClass = new Array();


    //--------------------------------------------------
    // Validation Functions
    //--------------------------------------------------      

	//--------------------------------------------------
	// Default Error Message
	//--------------------------------------------------
	jQuery.each(jQuery.validator.messages, function(i) {
		jQuery.validator.messages[i] = "<p class=\"error\">Please fill the field</p>";
	});

	//--------------------------------------------------
	// Time Selection Validation
	//--------------------------------------------------
	jQuery.validator.addMethod( 
		"selectNone", 
		function(value, element) { 
			if (element.value == "none") { 
				return false; 
			} 
			else return true; 
		}, 
		"<p class=\"error\">Please select a time</p>"); 
	
	//--------------------------------------------------
	// Days of Week validation
	//--------------------------------------------------
	jQuery.validator.addMethod( 
		"daysRequired", 
		function(value, element) { 
			var checkedCount = 0;
			jQuery(element).parent().parent().children().children('.daysRequired:checked').each( function() {
				checkedCount++;
			});
			if (checkedCount === 0) { 
				return false; 
			} 
			else return true; 
		}, 
		"<p class=\"error\">Select a day!</p>"); 

/**
 * Class name validation: only require a class name if it has at least
 * one section. Backend throws out empty classes and it's more
 * convenient if we can let the user have one extra, empty class. This
 * is because we automatically add a new class each time we do an
 * autofill to make the life of the user easier and less confusing.
 */
jQuery.validator.addMethod('classRequired',
			   function(value, element)
			   {
			       if (value.length)
				   return true;

			       var css_classes = jQuery(element).attr('class');
			       var cnum_pos = css_classes.indexOf('className');
			       var cnum = css_classes.substr(cnum_pos + 'className'.length, css_classes.indexOf(' ', cnum_pos) - cnum_pos - 'className'.length) * 1;
			       if (cnum < 0 || cnum > classNum)
				   alert('JS error: ' + cnum + ' is an invalid class number.');

			       /*
				* ignore the class with no
				* sections. This only works when the
				* class was added and the user _never_
				* clicked the Add Section button. Once
				* the user clicks that button, he has
				* to delete the class because of how
				* our numbering works.
				*/
			       if (!sectionsOfClass[cnum])
				   return true;

			       return false;
			   },
			   '<p class="error">Enter Class Name.</p>');

	//--------------------------------------------------
	// Add validation rules
	//--------------------------------------------------
	jQuery.validator.addClassRules("selectRequired", {
		selectNone: true
	});
	jQuery.validator.addClassRules("daysRequired", {
		daysRequired: true
	});
jQuery.validator.addClassRules('classRequired', { classRequired: true });


    //--------------------------------------------------
    // General Input Functions
    //--------------------------------------------------

/**
 * \brief
 * Returns the common inputs for each new section.
 */
function genSectionHtml(cnum)
{
    genSectionHtml_n(cnum, '', '', '', '', '', '', '');
}

/* @TODO: This should select & set items based on args, if the args != '' */
function genSectionHtml_n(cnum, name, synonym, stime, etime, days, prof, location, type)
{
		var snum = sectionsOfClass[cnum];
		
		var cssclasses = 'section class' + cnum;
		if(type == 'lab') {
		    cssclasses += ' lab';
		}
		
		var result = '<tr class="' + cssclasses + '"><td class="none"></td>';
	        result = result + '<td class="sectionIdentifier center"><input type="text" size="1" class="required" name="postData[' + cnum + '][' + snum + '][letter]" value="' + name + '" /><input type="hidden" name="postData[' + cnum + '][' + snum + '][synonym]" value="' + synonym + '" /></td>';
		result = result + '<td class="professor center"><input type="text" size="10" class="profName" name="postData[' + cnum + ']['+ snum + '][professor]" value="' + prof + '" /></td>';
		result = result + '<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][start]"><option value="none"></option>' +
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
			if (stime_end != '00' && stime_end != '30')
			    result = result + genOptionHtml(stime, prettyTime(stime), stime);
		    }

		result = result + '</select></td>\
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
		    genOptionHtml("2120", "9:20 pm", etime);

		if (etime.length > 0)
		    {
			var etime_end = etime.substr(2);
			if (etime_end != '50' && etime_end != '20')
			    result = result + genOptionHtml(etime, prettyTime(etime), etime);
		    }

		result = result + '</select></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][0]" value="1" ' + (days.m ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][1]" value="1" ' + (days.t ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][2]" value="1" ' + (days.w ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][3]" value="1" ' + (days.h ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][4]" value="1" ' + (days.f ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][5]" value="1" ' + (days.s ? 'checked="checked"' : '') + ' /></td>';
		result = result + '<td class="removeCell"><div class="deleteSection"><input type="button" value="x" class="gray" /></div></td><td class="emptyCell">' +
		    '<input type="hidden" name="postData[' + cnum + '][' + snum + '][location]" value="' + location + '" />' +
		    '<input type="hidden" name="postData[' + cnum + '][' + snum + '][type]" value="' + type + '" />' +
		    '</td></tr>';
		return result;
	}

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
  var td = tr.eq(tr.length-2);

  jQuery('td:first', td).qtip({
    /* slate_permutate_example_course_id is set globally in input.php. */
    content: 'Start typing your class ID (such as ' + slate_permutate_example_course_id + ') and click a suggestion to add sections',
    style: {
      border: { 
        width: 3,
        radius: 4,
        color: '#333'
      },
      name: 'dark',
      tip: true
    },
    show: { effect: { type: 'fade', length: 2000 } },
    show: { ready: true }, 
/*    hide: { when: { event: 'inactive' } }, */
    corner: { target: 'topMiddle', tooltip: 'bottomMiddle' }
  });
}

/**
 * \brief
 *   Add a section to a class.
 */
function add_section_n(cnum, name, synonym, stime, etime, days, prof, location, type)
{
    jQuery('.pclass'+cnum).after(genSectionHtml_n(cnum, name, synonym, stime, etime, days, prof, location, type));
    sectionsOfClass[cnum] ++;

    /* unhide the saturday columns if it's used by autocomplete data */
    if (days.s)
	jQuery('#jsrows col.saturday').removeClass('collapsed');
}
function add_section(cnum)
{
    return add_section_n(cnum, '', '', '', '', {m: false, t: false, w: false, h: false, f: false, s: false}, '', '', '');
}

/**
 * Add a list of sections gotten via an AJAX call.
 */
function add_sections(cnum, data)
{
    var i;
    if (!data.sections)
	return;
    /*
     * we get the sections in the correct order. For the user to see
     * them in the correct order, we must reverse the add_setion_n()
     * calls.
     */
    for (i = data.sections.length - 1; i >= 0; i --)
	{
	    section = data.sections[i];
	    add_section_n(cnum, section.section, section.synonym, section.time_start, section.time_end, section.days, section.prof, section.location, section.type);
	}
}

	//--------------------------------------------------
	// Adds a new class to the input.
	//--------------------------------------------------
	function add_class_n(name)
	{
		sectionsOfClass[classNum] = 0; // Initialize at 0
		jQuery('#jsrows').append('<tr id="tr-course-' + classNum + '" class="class class' + classNum + ' pclass' + classNum + '"><td class="nameTip"><input type="text" id="input-course-' + classNum + '" class="classRequired defText className'+classNum+' className" title="Class Name" name="postData[' + classNum + '][name]" value="' + name + '" /></td><td colspan="10"></td><td class="tdInput"><div class="addSection"><input type="button" value="+" class="gray" /></div></td><td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" class="gray" /></div></td></tr>');

		/* store classNum as course_i into the <tr />: */
		jQuery('#tr-course-' + classNum).data({course_i: classNum});

		var class_elem = jQuery('.className' + classNum);
		class_elem.autocomplete({ source: "auto.php" });
		class_elem.bind('autocompleteselect', {class_num: classNum, class_elem: class_elem},
			function(event, ui)
			    {
				if (!ui.item)
				    return;

				if (ui.item.value.indexOf('-') != -1)
				    {
					jQuery.ajax(
						    {
							url: 'auto.php',
							    data: {'getsections': 1, 'term': ui.item.value},
							    context: {'class_num': event.data.class_num},
							    success: function(data, textStatus, reqobj)
							    {
								var new_course_num;

								if (data.sections)
								    {
									add_sections(this.class_num, data);
									new_course_num = add_class();

									/* position the user's cursor the new class's input box */
									jQuery('#input-course-' + new_course_num).focus();
								    }
							    }
						    }
						    );
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
function add_class()
{
    return add_class_n('');
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
    if (i_hour <= 12)
	{
	    m = 'a';
	}
    else
	{
	    m = 'p';
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
	// Validates the form (pre-submission check)
	//--------------------------------------------------
	jQuery('#scheduleForm').validate({ debug: false });

	//--------------------------------------------------
	// Bind the class-adding method
	//--------------------------------------------------
	jQuery('#addclass').click(function() {
		add_class();
	});

	//--------------------------------------------------
	// Deletes the selected class from input
	//--------------------------------------------------
	jQuery('.deleteClass').live('click', function() {
		if(confirm('Delete class and all sections of this class?')) {
		    jQuery('.class' + jQuery(this).parent().parent().data('course_i')).remove();
		}
	});

	//--------------------------------------------------
	// Deletes the selected section from the input
	//--------------------------------------------------
	jQuery('.deleteSection').live('click', function() {
	  // Decreases the total number of classes
		sectionsOfClass[jQuery(this).parent().parent().data('course_i')]--;
	  
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

	});

	//--------------------------------------------------
	// Bind the section-adding method
	//--------------------------------------------------
	jQuery('.addSection').live('click', function() {
		var course_i = jQuery(this).parent().parent().data('course_i');
		add_section(course_i, sectionsOfClass[course_i]);
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


});
