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

jQuery(document).ready( function()
  {
      jQuery('#show-box input').change(show_box_change);
      jQuery('#show-box input').change();
  }
);
