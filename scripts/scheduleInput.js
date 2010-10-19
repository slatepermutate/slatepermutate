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
		"<p class=\"error\">Please select a time</p>" 
	); 
	
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
			if (checkedCount == 0) { 
				return false; 
			} 
			else return true; 
		}, 
		"<p class=\"error\">Select a day!</p>" 
	); 

	//--------------------------------------------------
	// Add validation rules
	//--------------------------------------------------
	jQuery.validator.addClassRules("selectRequired", {
		selectNone: true
	});
	jQuery.validator.addClassRules("daysRequired", {
		daysRequired: true
	});



    //--------------------------------------------------
    // General Input Functions
    //--------------------------------------------------

	//--------------------------------------------------
	// Returns the common inputs for each new section.
	//--------------------------------------------------
        function genSectionHtml(cnum)
	{
		genSectionHtml_n(cnum,'','','','','','');
	}

	/* @TODO: This should select & set items based on args, if the args != '' */
	function genSectionHtml_n(cnum,name,synonym,stime,etime,days,prof)
	{
		var snum = sectionsOfClass[cnum];

		var result = '<tr class="section class' + cnum + '"><td class="none"></td>';
	        result = result + '<td class="sectionIdentifier center"><input type="text" size="1" class="required" name="postData[' + cnum + '][' + snum + '][letter]" value="' + name + '" /></td>';
		result = result + '<td class="professor center"><input type="text" size="10" class="required" name="postData[' + cnum + ']['+ snum + '][professor]" value="' + prof + '" /></td>';
		result = result + '<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][start]"><option value="none"></option>'
				 + genOptionHtml("0700", "7:00 am", stime) + genOptionHtml("0730", "7:30 am", stime)
				 + genOptionHtml("0800", "8:00 am", stime) + genOptionHtml("0830", "8:30 am", stime)
				 + genOptionHtml("0900", "9:00 am", stime) + genOptionHtml("0930", "9:30 am", stime)
				 + genOptionHtml("1000", "10:00 am", stime) + genOptionHtml("1030", "10:30 am", stime)
				 + genOptionHtml("1100", "11:00 am", stime) + genOptionHtml("1130", "11:30 am", stime)
				 + genOptionHtml("1200", "12:00 pm", stime) + genOptionHtml("1230", "12:30 pm", stime)
				 + genOptionHtml("1300", "1:00 pm", stime) + genOptionHtml("1330", "1:30 pm", stime)
				 + genOptionHtml("1400", "2:00 pm", stime) + genOptionHtml("1430", "2:30 pm", stime)
				 + genOptionHtml("1500", "3:00 pm", stime) + genOptionHtml("1530", "3:30 pm", stime)
				 + genOptionHtml("1600", "4:00 pm", stime) + genOptionHtml("1630", "4:30 pm", stime)
				 + genOptionHtml("1700", "5:00 pm", stime) + genOptionHtml("1730", "5:30 pm", stime)
				 + genOptionHtml("1800", "6:00 pm", stime) + genOptionHtml("1830", "6:30 pm", stime)
				 + genOptionHtml("1900", "7:00 pm", stime) + genOptionHtml("1930", "7:30 pm", stime)
				 + genOptionHtml("2000", "8:00 pm", stime) + genOptionHtml("2030", "8:30 pm", stime)
		    + genOptionHtml("2100", "9:00 pm", stime);

		if (stime.length > 0)
		    {
			var stime_end = stime.substr(2);
			if (stime_end != '00' && stime_end != '30')
			    result = result + genOptionHtml(stime, prettyTime(stime), stime);
		    }

		result = result + '</select></td>\
			<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][end]"><option value="none"></option>'
				 + genOptionHtml("0720", "7:20 am", etime) + genOptionHtml("0750", "7:50 am", etime)
				 + genOptionHtml("0820", "8:20 am", etime) + genOptionHtml("0850", "8:50 am", etime)
				 + genOptionHtml("0920", "9:20 am", etime) + genOptionHtml("0950", "9:50 am", etime)
				 + genOptionHtml("1020", "10:20 am", etime) + genOptionHtml("1050", "10:50 am", etime)
				 + genOptionHtml("1120", "11:20 am", etime) + genOptionHtml("1150", "11:50 am", etime)
				 + genOptionHtml("1220", "12:20 pm", etime) + genOptionHtml("1250", "12:50 pm", etime)
				 + genOptionHtml("1320", "1:20 pm", etime) + genOptionHtml("1350", "1:50 pm", etime)
				 + genOptionHtml("1420", "2:20 pm", etime) + genOptionHtml("1450", "2:50 pm", etime)
				 + genOptionHtml("1520", "3:20 pm", etime) + genOptionHtml("1550", "3:50 pm", etime)
				 + genOptionHtml("1620", "4:20 pm", etime) + genOptionHtml("1650", "4:50 pm", etime)
				 + genOptionHtml("1720", "5:20 pm", etime) + genOptionHtml("1750", "5:50 pm", etime)
				 + genOptionHtml("1820", "6:20 pm", etime) + genOptionHtml("1850", "6:50 pm", etime)
				 + genOptionHtml("1920", "7:20 pm", etime) + genOptionHtml("1950", "7:50 pm", etime)
				 + genOptionHtml("2020", "8:20 pm", etime) + genOptionHtml("2050", "8:50 pm", etime)
		    + genOptionHtml("2120", "9:20 pm", etime);

		if (etime.length > 0)
		    {
			var etime_end = etime.substr(2);
			if (etime_end != '50' && etime_end != '20')
			    result = result + genOptionHtml(etime, prettyTime(etime), etime);
		    }

		result = result + '</select></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][0]" value="1" ' + (days.m ? 'checked="checked"' : '') + ' /></td> \
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][1]" value="1" ' + (days.t ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][2]" value="1" ' + (days.w ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][3]" value="1" ' + (days.h ? 'checked="checked"' : '') + ' /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][4]" value="1" ' + (days.f ? 'checked="checked"' : '') + ' /></td>';
		result = result + '<td><div class="deleteSection"><input type="button" value="x" class="gray" /></div></td><td></td></tr>';
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

        //--------------------------------------------------
        // Add a section to a class
        //--------------------------------------------------
	function add_section_n(cnum,name,synonym,stime,etime,days,prof)
	{
		 jQuery('.pclass'+cnum).after(genSectionHtml_n(cnum,name,synonym,stime,etime,days,prof));
		 sectionsOfClass[cnum] ++;
	}
        function add_section(cnum)
	{
	    return add_section_n(cnum, '', '', '', '', {'m':false, 't':false, 'w':false, 'h':false, 'f':false}, '');
        }

/**
 * Add a list of sections gotten via an AJAX call.
 */
function add_sections(cnum, data)
{
    var i;
    if (!data.sections)
	return;
    for (i = data.sections.length - 1; i >= 0; i --)
	{
	    section = data.sections[i];
	    add_section_n(cnum, section.section, section.synonym, section.time_start, section.time_end, section.days, section.prof);
	}
}

	//--------------------------------------------------
	// Adds a new class to the input.
	//--------------------------------------------------
	function add_class_n(name)
	{
		sectionsOfClass[classNum] = 0; // Initialize at 0
		jQuery('#jsrows').append('<tr title="' + classNum + '" class="class class' + classNum + ' pclass' + classNum + '"><td><input type="text" class="className required defText className'+classNum+'" title="Class Name" name="postData[' + classNum + '][name]" value="' + name + '" /></td><td colspan="9"></td><td class="tdInput"><div class="addSection"><input type="button" value="+Section" class="gray" /></div></td><td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" class="gray" /></div></td></tr>');
		jQuery('.className' + classNum).autocomplete({
			source: "auto.php",
		});
		jQuery('.className' + classNum).bind('autocompleteselect', {'class_num': classNum},
			function(event, ui)
			    {
				if (ui.item && ui.item.value.indexOf('-'))
				    {
					jQuery.ajax(
						      {
							  url: 'auto.php',
							  data: {'getsections': 1, 'term': ui.item.value},
							  context: {'class_num': event.data.class_num},
							  success: function(data, textStatus, reqobj)
							      {
								  add_sections(this.class_num, data);
							      }
						      }
						      );
				    }
			    });


		classNum++;

		return (classNum - 1);
	};
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
		jQuery('#scheduleForm').validate({
			debug: false,
		}); 

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
			jQuery('.class'+ jQuery(this).parent().parent().attr("title")).remove();
		}
	});

	//--------------------------------------------------
	// Deletes the selected section from the input
	//--------------------------------------------------
	jQuery('.deleteSection').live('click', function() {
		sectionsOfClass[jQuery(this).parent().parent().attr("title")]--; // Decreases the number of classes
		jQuery(this).parent().parent().remove();
	});

	//--------------------------------------------------
	// Bind the section-adding method
	//--------------------------------------------------
	jQuery('.addSection').live('click', function() {
		add_section(jQuery(this).parent().parent().attr("title"), sectionsOfClass[jQuery(this).parent().parent().attr("title")]);
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
	    if (jQuery(this).val() == "")
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
});
