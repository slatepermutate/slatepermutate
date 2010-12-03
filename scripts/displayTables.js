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

jQuery(document).ready( function()
  {
      jQuery('#show-box input').change(show_box_change);
      jQuery('#show-box input').change();
  }
);
