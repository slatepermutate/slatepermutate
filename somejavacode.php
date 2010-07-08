//**************************************************
// Classes.java			Author: Nathan Gelderloos
//
// Represents a class.
//**************************************************

    public class Classes
   {
      private String name;
      private Section[] sections;
      private int nsections;
      
      private boolean DEBUG = false;
   
       public Classes(String n)
      {
         name = n;
         sections = new Section[1];
         nsections = 0;
      }
   
   // Adds a new section to the class.
       public void addSection(String l, int s, int e, int d)
      {
         checkSections();
         sections[nsections] = new Section(l, s, e, d);
         nsections++;
      }
      
   // Makes sure there is still room in the array.
   // This method should be called before
   //   anything is added to the array.
       private void checkSections()
      {
         if(sections.length == nsections)
         {
            Section[] result = new Section[nsections+1];
            for(int i = 0; i < sections.length; i++)
            {
               result[i] = sections[i];
            }
            sections = result;
         }
      }
   
   // Returns the number of sections in the class.
       public int getnsections()
      {
         return nsections;
      }
   
   // Returns the desired section for analysis.
       public Section getSection(int i)
      {
         Section result = sections[i];
         return result;
      }
      
       public int getStartTime(int i)
      {
         Section temp = sections[i];
         return sections[i].getStartTime();
      }
   
   // Sets the DEBUG variable to the desired setting.
       public void setDEBUG(boolean debugger)
      {
         DEBUG = debugger;
      }
   
   // Returns the name of the class.
       public String getName()
      {
         return name;
      }
   
   }
 