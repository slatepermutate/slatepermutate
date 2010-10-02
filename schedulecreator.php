<?php

include_once 'class.schedule.php';
include_once 'class.class.php';
include_once 'class.section.php';

//**************************************************
// ScheduleCreator.java	Author: Nathan Gelderloos
// 
// Creates a list of classes.
//**************************************************

   
         $allClasses = new Schedule("Fall 2010");
  	
         $allClasses->addClass("CS 104");
         $allClasses->addSection("CS 104", "A", 1030, 1120, 24);
      	
         $allClasses->addClass("Engr 209");
         $allClasses->addSection("Engr 209", "A", 1330, 1420, 1235);
         $allClasses->addSection("Engr 209", "B", 1430, 1520, 1235);
      	
         $allClasses->addClass("Engr 209L");
         $allClasses->addSection("Engr 209L", "A", 1830, 2120, 4);
         $allClasses->addSection("Engr 209L", "B", 1830, 2120, 3);
      	
         $allClasses->addClass("Math 231");
         $allClasses->addSection("Math 231", "A", 800, 850, 1235);
      	
         $allClasses->addClass("Phys 235");
         $allClasses->addSection("Phys 235", "A", 900, 950, 135);
         $allClasses->addSection("Phys 235", "B", 1130, 1220, 135);
         $allClasses->addSection("Phys 235", "C", 1230, 1320, 135);
      	
         $allClasses->addClass("Phys 235L");
         $allClasses->addSection("Phys 235L", "A", 1430, 1720, 1);
         $allClasses->addSection("Phys 235L", "B", 1430, 1720, 3);
         $allClasses->addSection("Phys 235L", "C", 830, 1120, 4);
         $allClasses->addSection("Phys 235L", "D", 1230, 1520, 4);
      	
         $allClasses->addClass("Rel 131");
         $allClasses->addSection("Rel 131", "A", 800, 850, 135);
         $allClasses->addSection("Rel 131", "B", 900, 950, 135);
         $allClasses->addSection("Rel 131", "C", 1330, 1420, 135);
         $allClasses->addSection("Rel 131", "D", 1430, 1520, 135);
         $allClasses->addSection("Rel 131", "E", 835, 950, 24);
         $allClasses->addSection("Rel 131", "F", 1030, 1145, 24);
         $allClasses->addSection("Rel 131", "G", 1205, 1320, 24);
         $allClasses->addSection("Rel 131", "H", 1330, 1445, 24);
      	       
         $allClasses->findPossibilities();
         $allClasses->writeoutTables();
