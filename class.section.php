<?php

include_once 'errors.php';

//--------------------------------------------------
// class.section.php  Author:Nathan Gelderloos
//
// Represents a section of a class.
//--------------------------------------------------
   
class Section {

   private $letter;	// Section letter
   private $prof;	// Professor
   private $start;	// Start time
   private $tend;	// End time
   private $idays;	// Integer version of meeting days
   private $bdays;	// Boolean array of meeting days

function __construct ($l, $p, $s, $e, $d)
{
   $this->letter = $l;
   $this->prof = $p;
   $this->start = $s;
   $this->tend = $e;
   $this->idays = $d;
   $this->bdays = $this->setbdays();
}

function setbdays()
{
   $result;

   if($this->idays == 12345)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = true;}

   if($this->idays == 1234)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = false;}
   if($this->idays == 1235)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = true;}
   if($this->idays == 1245)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = true;}
   if($this->idays == 1345)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = true;}
   if($this->idays == 2345)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = true;}

   if($this->idays == 123)
      {$result[0] = true; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = false;}
   if($this->idays == 124)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = false;}
   if($this->idays == 125)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = true;}
   if($this->idays == 134)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = false;}
   if($this->idays == 135)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = true;}
   if($this->idays == 145)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = true;}
   if($this->idays == 234)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = true; $result[4] = false;}
   if($this->idays == 235)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = true;}
   if($this->idays == 245)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = true;}
   if($this->idays == 345)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = true;}

   if($this->idays == 12)
      {$result[0] = true; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = false;}
   if($this->idays == 13)
      {$result[0] = true; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = false;}
   if($this->idays == 14)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = false;}
   if($this->idays == 15)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = true;}
   if($this->idays == 23)
      {$result[0] = false; $result[1] = true; $result[2] = true; $result[3] = false; $result[4] = false;}
   if($this->idays == 24)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = true; $result[4] = false;}
   if($this->idays == 25)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = true;}
   if($this->idays == 34)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = true; $result[4] = false;}
   if($this->idays == 35)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = true;}
   if($this->idays == 45)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = true;}
      
   if($this->idays == 1)
      {$result[0] = true; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = false;}
   if($this->idays == 2)
      {$result[0] = false; $result[1] = true; $result[2] = false; $result[3] = false; $result[4] = false;}
   if($this->idays == 3)
      {$result[0] = false; $result[1] = false; $result[2] = true; $result[3] = false; $result[4] = false;}
   if($this->idays == 4)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = true; $result[4] = false;}
   if($this->idays == 5)
      {$result[0] = false; $result[1] = false; $result[2] = false; $result[3] = false; $result[4] = true;}
         
   return $result;
}

function getLetter()
{
   return $this->letter;
}

function getProf()
{
   return $this->prof;
}

function getStartTime()
{
   return $this->start;
}

function getEndTime()
{
   return $this->tend;
}

function getM()
{
   return $this->bdays[0];
}

function getTu()
{
   return $this->bdays[1];
}

function getW()
{
   return $this->bdays[2];
}

function getTh()
{
   return $this->bdays[3];
}

function getF()
{
   return $this->bdays[4];
}

}

?>