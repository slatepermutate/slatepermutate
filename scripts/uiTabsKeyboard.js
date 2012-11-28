/* -*- tab-width: 4; -*-
 * Copyright 2012 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
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

jQuery(document).ready( function() {
	var share_facebook_a_obj = jQuery('#share-fb-a');
	var share_url_em_obj = jQuery('#share-url-em');

  jQuery(document).keydown(function(e) {

    var direction = null;

   // handle cursor keys
   if (e.keyCode == 37) {
     // slide left
     direction = 'prev';
   } else if (e.keyCode == 39) {
     // slide right
     direction = 'next';
   }
   if (direction != null) {
     var totaltabs = jQuery('#tabs').tabs('length'); //gettting the total # of tabs
     var selected = jQuery('#tabs').tabs('option', 'selected');//getting the currently selected tab
 
     if (direction == 'next') {
       if (selected <= totaltabs - 1)
       jQuery('#tabs').tabs('select',selected + 1)
    }
    else {
      if (selected != 0)
      jQuery('#tabs').tabs('select',selected - 1)
    }
  }
 });

	function share_update_uris(hash)
	{
		share_facebook_a_obj.attr('href', share_facebook_template + hash.replace('#', '%23'));
		share_url_em_obj.text(share_url_template + hash);		
	}
	share_update_uris(window.location.hash);

	jQuery('#tabs').bind('tabsselect', function(event, ui) {
		window.location.hash = ui.tab.hash;
		/*
		 * Update some links to be specific to the current bug (bug
		 * #92).
		 */
		share_update_uris(ui.tab.hash);
	});
});
