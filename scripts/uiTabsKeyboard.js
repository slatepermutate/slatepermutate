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
