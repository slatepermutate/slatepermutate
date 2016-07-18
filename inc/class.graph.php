<?php /* -*- mode: php; -*- */

class barGraph {

// This array of values is just here for the example.
private $values = array();
private $image = null;

// Pass me a multidimensional array with 'count', the size, and 'label', the label
public function __construct($userVals, $width=600, $height=300) {

  $this->values = $userVals;


  // Get the total number of columns we are going to plot
  $columns  = count($this->values);
  if($columns == 0) {
    throw new Exception("No input data!");
  }
  if($width < 5 || $height < 5) {
    throw new Exception("Image size too small!");
  }

  // Set the amount of space between each column
  $padding = 5; 

  // Get the width of 1 column
  $column_width = $width / $columns ;

  // Generate the image variables
  $im        = imagecreate($width,$height);
  $gray      = imagecolorallocate ($im,0xcc,0xcc,0xcc);
  $gray_lite = imagecolorallocate ($im,0xee,0xee,0xee);
  $gray_dark = imagecolorallocate ($im,0x7f,0x7f,0x7f);
  $white     = imagecolorallocate ($im,0xff,0xff,0xff);
  $black     = imagecolorallocate($im, 0, 0, 0);
    
  // Fill in the background of the image
  imagefilledrectangle($im,0,0,$width,$height,$white);
    
  $maxv = 0;

  // Calculate the maximum value we are going to plot
  for($i=0;$i<$columns;$i++)
    $maxv = max($this->values[$i]['count'], $maxv);

  // Now plot each column 
  for($i=0;$i<$columns;$i++) {
    $column_height = ($height / 100) * (( $this->values[$i]['count'] / $maxv) *100);
    $x1 = $i*$column_width;
    $y1 = $height-$column_height;
    $x2 = (($i+1)*$column_width)-$padding;
    $y2 = $height;

    imagefilledrectangle($im,$x1,$y1,$x2,$y2,$gray);

    $label = $this->values[$i]['label'];
    $font = '/usr/share/fonts/liberation/LiberationSans-Regular.ttf';
    $font2 = '/usr/share/fonts/liberation/LiberationSans-Bold.ttf';

    imagettftext($im, 10, 0, $x1 + ($column_width / 2) - 20, $height - 5, $black, $font, $label);
    imagettftext($im, 7, 0, $x1 + ($column_width / 2) - 7, $height - 20, $gray_dark, $font, $this->values[$i]['count']);
 
  }
  imagettftext($im, 10, 0, 0, 10, $black, $font2, "Past Month:");

  // Send the PNG header information. Replace for JPEG or GIF or whatever
  $this->image = $im;

  header ("Content-type: image/png");
  imagepng($this->image);
}

}
