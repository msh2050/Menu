/*
* Menu.h
* Author : Jacques Bellavance
* Date : Septembet 12, 2016
* Version : 1.00
*/

/*
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General
Public License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA  02111-1307  USA
*/

/*
 * Menu.h
 * Allows the use of menus in sketches.
 * The menu is declared in a String. 
 * (See Menu::parse() in Menu.cpp for instructions on how a menu is built) 
 * The menu is parsed, allowing easy navigation thru the menu
 */
 
#ifndef Menu_h
#define Menu_h

class Menu {
  public: //===================================================================================================
  //Constructor 
    Menu(String items);                                           //items : the String containing the menu

  //Methods
    //To be used in the setup part of the sketch
		void defineLcd(int columns, int rows);                        //The number of columns and rows of the LCD
		void mapKeyChar(char UP, char DOWN, char LEFT, char RIGHT);   //For keypads that returns chars
		void mapKeyInt(int UP, int DOWN, int LEFT, int RIGHT);        //For keypads that returns integers

    //Display the menu on the LCD tools 
		String lcdLine(int line);          														//Returns the label to be displayed on the LCD's "line"
		bool LcdNeedsUpdate();                                        //Returns "true" if the LCD needs to be updated
		void LcdUpdated();                                            //Says that the current menu was udated on the LCD
		void updateLcd();                                             //Says that the LCD will need to be updated

    //A key was pressed, update the menu
		int update(int key);	                                        //Update the menu according to "mapKeyInt"
		int update(char key);                                         //Update the menu according to "mapKeyChar"

    //Provide some informations to the sketch 
	  String getCurrentLabel();                                     //Returns the label of the current item
		int getCurrentItem();                                         //Returns the number of the current menu item (currentNode)
    int getAction();            	                                //Returns the action associated to the current item 
		int itemNumber(String find);																	//Returns the number of the first menu item with label "find"
		void selectItem(int number);																	//Makes item "number" the current item

    //Let the Sketch advise us that
		void done();                                                  //The action is handled, return to the menu
    void reset();                                                 //Sets currentNode to 1

  private: //====================================================================================================
    //Default values for the size of the LCD
    int LCDcol = 16;
    int LCDrows = 2;

    //The full menu is contained in "MYitems"
    //A node is associated to each item in the menu.
    //The nodes are placed in the table "nodes[]"
    //The menu is then parsed to setup nodes
    String MYitems;     //The full menu
    struct node {       //For each item :
      int starts = 0;     //the index of the start of the label
      int ends = 0;       //the index of the end of the label
      byte parent = 0;    //the node number of the parent of this item
      byte eldest = 1;    //the node number of the eldest of this item
      int action = 0;     //the action associated to this item
    };
	  node *nodes;              //The table that holds the nodes (using calloc() to use only the needed memory)
    void menuParse();         //Actual parsing of "MYitems" and setup of "nodes[]"
		int updateMenu(int key);	//Update the menu
		bool needsUpdate;         //The flag to signal that the LCD needs an update or not

    //Labels of the menu or submenu to be displayed on the LCD
    String label(int node); //Returns the label of "node"

    //Moving around the menus
    int currentNode = 1;            //The index of the current node
    int lastNode = 1;               //The index of the last node
    int parent(int node);           //The parent of "node"
    int eldest(int node);           //The eldest child of "node"
    int previousSibling(int node);  //The previous sibling of "node"
    int nextSibling(int node);      //The next sibling of "node"
		int siblingsCount(int node);    //The number of siblings of "node"
    int rank(int node);             //The rank of "node" amongst it's siblings
    

  //SWITCHES  
		int UP = 1;         //The values that updateMenu(int key) is expecting
		int DOWN = 2;
		int LEFT = 3;
		int RIGHT = 4;
		
    int MYUP;           //The values that will be provided by the sketch via update(int key)
    int MYDOWN;
    int MYLEFT;
    int MYRIGHT;

		char MYCHARUP;      //The values that will be provided by the sketch via update(char key)
		char MYCHARDOWN;
		char MYCHARLEFT;
		char MYCHARRIGHT;
};

#endif
