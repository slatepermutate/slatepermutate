
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
			if (element.value == "none") 
			{ 
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
			if (checkedCount == 0) 
			{ 
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


    jQuery(document).ready(function() {
	//--------------------------------------------------
	// Validates the form (pre-submission check)
	//--------------------------------------------------
		jQuery('#scheduleForm').validate({
			debug: false,
		}); 


	/* classNum is declared in the <head/> to enable loading of saved classes */
	/* sectionsOfClass is declared in the <head/> to enable loading of saved sections */
        function numberedIds(name){
		return '<td class="sectionIdentifier">\
                                <select name="'+name+'"><option value="-">-</option><option value="1">1</option><option value="2">2</option>\
                                <option value="3">3</option><option value="4">4</option><option value="5">5</option><option value="6">6</option><option value="7">7</option>\
                                <option value="8">8</option><option value="9">9</option><option value="10">10</option><option value="11">11</option></select></td>\
                                </select></td>';
        }
	function letteredIds(name){
		return '<td class="sectionIdentifier">\
                                <select name="'+name+'"><option value="-">-</option><option value="A">A</option><option value="B">B</option>\
                                <option value="C">C</option><option value="D">D</option><option value="E">E</option><option value="F">F</option><option value="G">G</option>\
                                <option value="H">H</option><option value="I">I</option><option value="J">J</option><option value="K">K</option></select></td>\
                                </select></td>';
	}
	function customIds(name){
		return '<td class="sectionIdentifier center"><input type="text" size="1" class="required" name="' + name + '" /></td>';
	}

	//--------------------------------------------------
	// Returns the common inputs for each new section.
	//--------------------------------------------------
	function getCommonInputs(cnum) {
		var snum = sectionsOfClass[cnum];

		var result = '';
		if(jQuery('#isNumeric').val() == "lettered"){
			result = result + letteredIds('postData[' + cnum + '][' + snum + '][letter]');
		}
		else if(jQuery('#isNumeric').val() == "numbered") {
                        result = result + numberedIds('postData[' + cnum + '][' + snum + '][letter]');
		}
		else {
                        result = result + customIds('postData[' + cnum + '][' + snum + '][letter]');
		}

		result = result + '<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][start]"><option value="none"></option>\
				<option value="700">7:00 am</option><option value="730">7:30 am</option>\
				<option value="800">8:00 am</option><option value="830">8:30 am</option>\
				<option value="900">9:00 am</option><option value="930">9:30 am</option>\
				<option value="1000">10:00 am</option><option value="1030">10:30 am</option>\
				<option value="1100">11:00 am</option><option value="1130">11:30 am</option>\
				<option value="1200">12:00 pm</option><option value="1230">12:30 pm</option>\
				<option value="1300">1:00 pm</option><option value="1330">1:30 pm</option>\
				<option value="1400">2:00 pm</option><option value="1430">2:30 pm</option>\
				<option value="1500">3:00 pm</option><option value="1530">3:30 pm</option>\
				<option value="1600">4:00 pm</option><option value="1630">4:30 pm</option>\
				<option value="1700">5:00 pm</option><option value="1730">5:30 pm</option>\
				<option value="1800">6:00 pm</option><option value="1830">6:30 pm</option>\
				<option value="1900">7:00 pm</option><option value="1930">7:30 pm</option>\
				<option value="2000">8:00 pm</option><option value="2030">8:30 pm</option>\
				<option value="2100">9:00 pm</option></select></td>\
			<td><select class="selectRequired" name="postData[' + cnum + '][' + snum + '][end]"><option value="none"></option>\
				<option value="720">7:20 am</option><option value="750">7:50 am</option>\
				<option value="820">8:20 am</option><option value="850">8:50 am</option>\
				<option value="920">9:20 am</option><option value="950">9:50 am</option>\
				<option value="1020">10:20 am</option><option value="1050">10:50 am</option>\
				<option value="1120">11:20 am</option><option value="1150">11:50 am</option>\
				<option value="1220">12:20 pm</option><option value="1250">12:50 pm</option>\
				<option value="1320">1:20 pm</option><option value="1350">1:50 pm</option>\
				<option value="1420">2:20 pm</option><option value="1450">2:50 pm</option>\
				<option value="1520">3:20 pm</option><option value="1550">3:50 pm</option>\
				<option value="1620">4:20 pm</option><option value="1650">4:50 pm</option>\
				<option value="1720">5:20 pm</option><option value="1750">5:50 pm</option>\
				<option value="1820">6:20 pm</option><option value="1850">6:50 pm</option>\
				<option value="1920">7:20 pm</option><option value="1950">7:50 pm</option>\
				<option value="2020">8:20 pm</option><option value="2050">8:50 pm</option>\
				<option value="2120">9:20 pm</option></select></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][0]" value="1" /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][1]" value="1" /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][2]" value="1" /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][3]" value="1" /></td>\
			<td class="cbrow"><input type="checkbox" class="daysRequired" name="postData[' + cnum + '][' + snum + '][days][4]" value="1" /></td>';
	
		return result;
	}

	//--------------------------------------------------
	// Adds a new class to the input.
	//--------------------------------------------------
	function addRow(){
		sectionsOfClass[classNum] = 0; // This is class 0, initialize at 0
		jQuery('#jsrows').append('<tr title="' + classNum + '" class="class class' + classNum + '"><td><input type="text" class="required defText" title="Class Name" name="postData[' + classNum + '][name]" /></td><td colspan="8"></td><td class="tdInput"><div class="addSection"><input type="button" value="Add section" class="gray" /></div></td><td class="tdInput"><div class="deleteClass"><input type="button" value="Remove" class="gray" /></div></td></tr>');
		classNum++;
	};
	
	addRow(); // Add initial row

	//--------------------------------------------------
	// Adds a new class when the add class button is 
	// clicked.
	//--------------------------------------------------
	jQuery('#addclass').click(function() {
		addRow();
	});

	//--------------------------------------------------
	// Deletes the selected class from input.
	//--------------------------------------------------
	jQuery('.deleteClass').live('click', function() {
		if(confirm('Delete class and all sections of this class?')) {
			jQuery('.class'+ jQuery(this).parent().parent().attr("title")).remove();
		}
	});

	//--------------------------------------------------
	// Deletes the selected section from the input.
	//--------------------------------------------------
	jQuery('.deleteSection').live('click', function() {
		sectionsOfClass[jQuery(this).parent().parent().attr("title")]--; // TODO: this only decreases the number of classes, so php should loop until this number of classes is found in the array
		jQuery(this).parent().parent().remove();
	});

	//--------------------------------------------------
	// Adds a section to the selected class.
	//--------------------------------------------------
	jQuery('.addSection').live('click', function() {
		sectionsOfClass[jQuery(this).parent().parent().attr("title")]++; // Increases sectionsOfClass[classNum]
		jQuery(this).parent().parent().after('<tr class="section class' + jQuery(this).parent().parent().attr("title") + '"><td class="none"></td>' + getCommonInputs(jQuery(this).parent().parent().attr("title")) + '<td><div class="deleteSection"><input type="button" value="x" class="gray" /></div></td><td></td></tr>');
	});

	//--------------------------------------------------
	// Resets the form
	//--------------------------------------------------
	jQuery('#reset').click(function() {
		jQuery('#scheduleForm').resetForm();
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
