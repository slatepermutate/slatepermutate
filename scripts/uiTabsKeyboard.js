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

jQuery(document).ready( function() {

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
});
