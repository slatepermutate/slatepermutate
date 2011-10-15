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

/**
 * \brief
 *   Handle changes to the input elements under #show-box.
 */
function show_box_change()
{
    var name = jQuery(this).attr('name');
    if (name && name.indexOf('-'))
	{
	    /* convert from 'show-prof' back to 'prof' */
	    var css_class = name.substr(name.indexOf('-') + 1);
	    if (jQuery('#' + name + ':checked').size())
		{
		    jQuery('.' + css_class).show();
		}
	    else
		{
		    jQuery('.' + css_class).hide();
		}
	}

    return false;
}

/**
 * \brief
 *   Do an AJAX loading of data with arbitrary error handling.
 *
 * \param target
 *   The jQuery object which should be populated with an error message
 *   or the result of loading.
 * \param data
 *   The data to send as a request. The school and semester keys shall
 *   automatically be set.
 * \param handler
 *   A function with the signature handler(target, data) which is called upon
 *   a successful response. There is a default handler which uses
 *   .html() to load the data.data.html into target.
 * \param error_handler
 *   A function with the signature handler(target, status_text, data)
 *   which is called upon an error. The default error_handler will
 *   store an error message in target, possibly provided by
 *   data.message if the HTTP request itself was successful but the
 *   server still claimed there is an error. The third argument, data,
 *   will be null if the error is at the HTTP level.
 */
function slate_permutate_load(target, data, handler, error_handler)
{
    if (jQuery.type(handler) == 'undefined')
	handler = function(target, data)
	    {
		target.html(data.html);
	    }

    if (!data.school)
	data.school = slate_permutate_school;
    if (!data.semester)
	data.semester = slate_permutate_semester;

    if (jQuery.type(error_handler) == 'undefined')
	error_handler = function(target, status_text, data)
	    {
		if (data)
		    if (data.message)
			target.html(data.message);
		    else
			target.html('<div class="error">Unknown error.</div>');
		else
		    target.html('<div class="error">HTTP error: ' + status_text + '</div>');
	    }

    jQuery.ajax({
                url: 'ajax.php',
		data: data,
		success: function(data, status_text, xhr)
		{
		    if (data && data.success && jQuery.type(data.data) != 'undefined')
			handler(target, data.data);
		    else
			error_handler(target, status_text, data);
		},
		dataType: 'json',
		error: function(xhr_jq, status_text, error)
		{
		    error_handler(target, status_text, null);
		},
		type: 'POST'
		});
}

jQuery(document).ready( function()
  {
      jQuery('#show-box input').change(show_box_change);
      jQuery('#show-box input').change();

      jQuery("#regDialog").dialog({ modal: true, width: 550, resizable: false, draggable: false, autoOpen: false });   
      jQuery('#regCodes').click( function() {
        jQuery('#regDialog-content').html('<p>Loading registration information...</p>');

	/* hmm... why isn't this information just stored in a global JS variable? */
	var tab_i = jQuery('#tabs').tabs('option','selected');
	var tab_fragment_i = /-([^-]+)$/.exec(jQuery('#the-tabs li:eq(' + tab_i + ') a').attr('href'))[1];
        var tab_course_data_json_selector = '#tabs-' + tab_fragment_i + ' .course-data';
	
        var tab_course_data_json = jQuery(tab_course_data_json_selector).text();
        var tab_course_data = eval('(' + tab_course_data_json + ')');

	slate_permutate_load(jQuery('#regDialog-content'), {school_registration_html: true, courses: tab_course_data});

        jQuery("#regDialog").dialog('open');

	
	
	return false;
      });

      jQuery('.qTipCell').qtip(
       {
          style: {
            tip: true,
            classes: "ui-tooltip-dark ui-tooltip-shadow ui-tooltip-rounded"
          },
          hide: {
            event: 'mouseleave click'
          },
          position:{
            my: 'bottom left',
            at: 'top center',
          }
      });  

      jQuery(".clicktoclipboard").click( function() {
        jQuery('.toclipboard', this).toggle();
      });
  }
);

