/*
 * Menu.cpp
 * Author : Jacques Bellavance
 * Date : September 12, 2016
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
 * Menu.cpp
 * Allows the use of menus in sketches.
 * The menu is declared in a String. 
 * (See menuParse() below for instructions on how a menu is built.) 
 * The menu is parsed, allowing easy navigation thru the menus.
 */
 
#include <Arduino.h>
#include <Menu.h>

//Constructor==================================================================================
Menu::Menu(String items) {
  MYitems = items;  //Full menu.
	//Find the number of items of the menu (count the colons) and allocate memory for the nodes.
  //Arduino's IDE reports the number of bytes used by the variables in the sketch.
  //Add 7 bytes * (items in your menu + 1) to get the actual space used.
	int count = 0;
	for (int i = 0; i < MYitems.length(); i++) if (MYitems.charAt(i) == ':') count++;
	nodes = (node*) calloc(count + 1, sizeof(node));

	menuParse();          //Parse "MYitems" in "nodes[]".
  currentNode = 1;      //Set first node as curent.
  needsUpdate = true;   //The LCD needs to be updated.
}//Constructor-----------------------------------------------------------------

 //menuParse====================================================================================================================
//The full menu is built in the sketch in the following mannner :
//  String menuItems = 
//  "-READ:000"
//  "--SENSORS:000"
//  "---SENSOR A1:101"
//  "---SENSOR A2:102"
//  "--SWITCHES:000"
//  "---SWITCH PIN 4:103"
//  "---SWITCH PIN 5:104"
//  "-SET:000"
//  "--SERVO ARM:105"
//  "--SERVO BASE:106"
//  "-MOVE SERVOS:107";
//The menu items contains 4 parts:
//  a series of dashes that indicate the item's level,
//  the label of the item,
//  a colon (:) (token),
//  a 3 digits number between "000" and "999" ("000" means: I have a submenu) to tag an action to be performed in the sketch.
//In order to navigate the menu, each item is associated to a node : 
//  struct node {       //For each item :
//    int starts = 0;     //The index of the start of the label in MYitems,
//    int ends = 0;       //The index of the end of the label in MYitems,
//    int parent = 0;     //The node number of the parent of this item,
//    int eldest = 1;     //The node number of the eldest child of this item,
//    int action = 0;     //The action associated to this item.
//  };
//-----------------------------------------------------------------------------------------------------------------------------
void Menu::menuParse() {
  int stack[8] = { 0,0,0,0,0,0,0,0 }; //Stack (Last-in/First-out) (parent management).
  int stackPtr = 7;                   //Stack pointer.
  int pos = 1;                        //The position of the pointer in the string "MYitems".
  int item = 1;                       //The pointer to the current item.
  int curLevel = 1;                   //The level of the current item.
  int nextLevel = 1;                  //The level of the next item.
  int len = MYitems.length();         //The length of "MYitems".

	nodes[0].eldest = 1;                                             //The first item in the menu is the eldest of nodes[0].
  while(pos < len) {                                               //Parse the whole menu.
    nodes[item].eldest = 1;                                          //Default value. "I have no child"
    nodes[item].starts = pos;                                        //The start of the label.
    while(MYitems.charAt(pos) != ':') pos++;                         //Forward to the ":" token.
    nodes[item].ends = pos;                                          //The end of the label.
    nodes[item].action = MYitems.substring(pos+1, pos+4).toInt();    //The integer associated to the action.
	  nodes[item].parent = stack[stackPtr];                            //The parent of the item (current on stack).
	  pos += 4;                                                        //Forward to the next item.
    nextLevel = 0 ;
    while(MYitems.charAt(pos) == '-') { pos++; nextLevel++; }        //Find the level of the next item (count dashes).
    if (nextLevel > curLevel) {                                      //If the next item has a higher level (item is a parent) :
		  stackPtr--; stack[stackPtr] = item;                              //Push the item as a parent on the stack.
      nodes[item].eldest = item + 1;                                   //The next item is the eldest child of the current item.
    }
    if (nextLevel < curLevel) {                                      //If the next item has a lower level :
		  for (int i = nextLevel; i < curLevel; i++) {                     //For the number of generations. 
		    stack[stackPtr] = 0;  stackPtr++;                                //Pull as many parents as necessary off the stack. 
      }                              
    }
    item++; curLevel = nextLevel;                                   //Go to next item.
  }
  lastNode = item-1;                                                //Set "lastNode" to the number of items in the menu.
}//menuParse-------------------------------------------------------------------------------------------------------------------

//label=====================================================================
//Return the label of the item "node".
//--------------------------------------------------------------------------
String Menu::label(int node) {
  return MYitems.substring(nodes[node].starts, nodes[node].ends);
}//label--------------------------------------------------------------------

//selectItem==========================
//Moves the pointer to node "number".
//------------------------------------
void Menu::selectItem(int number) {
	currentNode = number;
}//-----------------------------------

//parent===================================================
//Return the number of the parent of "node".
//---------------------------------------------------------
int Menu::parent(int node) {
    return nodes[node].parent;
}//parent--------------------------------------------------

//eldest============================================
//Return the number of the eldest child of "node".
//--------------------------------------------------
int Menu::eldest(int node) {
  return nodes[node].eldest;
}//eldest-------------------------------------------

//previousSibling===================================================
//If "node" is the eldest, return "node".
//Otherwise, return it's older sibling.
//------------------------------------------------------------------
int Menu::previousSibling(int node) {
  if (eldest(parent(node)) == node) return node;
  for (int i = node-1 ; i > 0 ; i--) {
    if (parent(i) == parent(node)) return i;
  }
}//previousSibling--------------------------------------------------

//nextSibling=====================================================
//Return the next sibling of "node",
//or "node" if it is the youngest.
//----------------------------------------------------------------
int Menu::nextSibling(int node) {
  for (int i = node+1 ; i <= lastNode ; i++) {
    if (parent(i) == parent(node)) return i;
  }//Passed this point, no older brother has been found.
  return node;
}//nextSibling----------------------------------------------------

//rank===========================================================
//Returs the rank of "node" amongst it's siblings.
//---------------------------------------------------------------
int Menu::rank(int node) {
	int child = eldest(parent(node)); //From the eldest child...
	int rankOfNode = 1;
  while (child != node) {
    child = nextSibling(child);
    rankOfNode++;
  }
  return rankOfNode;
}//rank----------------------------------------------------------

//siblingsCount========================================================================
//returns the number of siblings of "node".
//-------------------------------------------------------------------------------------
int Menu::siblingsCount(int node) {
  int parentOfNode = parent(node);
	int child = eldest(parentOfNode); //From the eldest child...
  int count = 0;
  for (int i = child ; i <= lastNode; i++) if (parent(i) == parentOfNode) count++;
  return count;
}//siblingsCount------------------------------------------------------------------------

 //getCurrentItem==========================================
 //Returns the to the current node.
 //---------------------------------------------------
int Menu::getCurrentItem() {
	return currentNode;
}//getCurrentItem-----------------------------------------

//getAction==========================================
//Returns the action associated to the current node.
//---------------------------------------------------
int Menu::getAction() {
  return nodes[currentNode].action;
}//getAction-----------------------------------------

//getCurrentLabel========================
//Returns the label of the current item.
//---------------------------------------
String Menu::getCurrentLabel() {
  return label(currentNode);
}//getCurrentLabel-----------------------

//done================================================================================================
//Allows the sketch to signal the library that the action is finished and that we return to the menu.
//----------------------------------------------------------------------------------------------------
void Menu::done() {
	needsUpdate = true;
}//done-----------------------------------------------------------------------------------------------

//restart==================
//Reinitialise the menu.
//-------------------------
void Menu::reset() {
  currentNode = 1;
}//restart-----------------

//updated===============================
//Signals that the LCD has been updated.
//--------------------------------------
void Menu::LcdUpdated() {
  needsUpdate = false;
}//updated------------------------------

//needsUpdate========================================
//Returns whether the LCD needs to be updated or not.
//---------------------------------------------------
bool Menu::LcdNeedsUpdate() {
  return needsUpdate;
}//needsUpdate---------------------------------------

//updateLcd==================================
//Signal that the LCD needs to be updated.
//-------------------------------------------
void Menu::updateLcd() {
	needsUpdate = true;
}//updateLcd---------------------------------

//defineLcd====================================================
//The number of columns and lines of the sketche's LCD.
//-------------------------------------------------------------
void Menu::defineLcd(int columns, int rows) {
	LCDcol = columns; //Number of columns of the sketche's LCD
	LCDrows = rows;   //Number of rows of the sketche's LCD
}//defineLcd---------------------------------------------------

//lcdLine==================================================================================================================================
//Return a string containing the label of the item to be displayed
//on the "requested Line" on the lcd for the current menu or submenu.
//Preceded by a caret ">" if the item is the current item.
//-----------------------------------------------------------------------------------------------------------------------------------------
String Menu::lcdLine(int requestedLine) {
	int currentRank = rank(currentNode);                              //Where the current node is amongst it's siblings.
  if((requestedLine + 1) > siblingsCount(currentNode)) return "";   //There is no item at this rank in the menu.
	int child = eldest(parent(currentNode));                          //From the eldest,
	int targetRank = requestedLine + 1;                               //Case where the LCD line 0 displays the eldest.
	if (currentRank >= LCDrows) targetRank += currentRank - LCDrows;  //If not, add the difference between the the item and LCD's line count.
	for (int i = 1; i < targetRank; i++)  child = nextSibling(child); //Find the item at "targetRank".
	if (currentRank == targetRank) return '>' + label(child);         //If it is the curent item, add a ">" before the label.
	else                           return ' ' + label(child);         //If not, add a " " before the label.
}//cdLine-----------------------------------------------------------------------------------------------------------------------------------

//mapKeyChar===========================================================
//This is used if the keypad returns characters.
//Most matrix keypads use this scheme.
//---------------------------------------------------------------------
void Menu::mapKeyChar(char UP, char DOWN, char LEFT, char RIGHT) {
	MYCHARUP = UP;
	MYCHARDOWN = DOWN;
	MYCHARLEFT = LEFT;
	MYCHARRIGHT = RIGHT;
}//mapKeyChars---------------------------------------------------------

 //mapKeyint-===========================================================
 //This is used if the keypad returns integers.
 //---------------------------------------------------------------------
void Menu::mapKeyInt(int UP, int DOWN, int LEFT, int RIGHT) {
	MYUP = UP;
	MYDOWN = DOWN;
	MYLEFT = LEFT;
	MYRIGHT = RIGHT;
}//mapKeyInt---------------------------------------------------------

//updateMenu=============================================================================
//Sets as current, the new item according to the key pressed.
//Returns the action associated to the item.
//UP, DOWN and LEFT sets the current node to, respectively:
//the previous sibling, the next sibling and the parent of the current node.
//For the RIGHT key, there are two possibilities :
//- The action associated to the node needs to be carried out by the sketch. (>"000")
//- The action is "000" and the current node becomes it's eldest child
//Raise the "needsUpdate" flag if the node changed.
//---------------------------------------------------------------------------------------
int Menu::updateMenu(int key) {
  int node = currentNode;
	if (key == UP) currentNode = previousSibling(currentNode);
	if (key == DOWN) currentNode = nextSibling(currentNode);
	if (key == LEFT && (parent(currentNode) != 0)) currentNode = parent(currentNode);
	if (key == RIGHT) {
    int action = getAction();
		if (action > 0) return action;
		else            if (eldest(currentNode) != 1) currentNode = eldest(currentNode);
	}
  if (currentNode != node) needsUpdate = true;
	return 0;
}//updateMenu-----------------------------------------------------------------------------

//update===========================================
//The Sketche's keypad returns integers.
//See Menu::mapKeyInt().
//See Menu::updateMenu() for more details.
//-------------------------------------------------
int Menu::update(int key) {
	if (key == 0) return 0;
	int action = 0;
	if (key == MYUP)  action = updateMenu(UP);
	if (key == MYDOWN)  action = updateMenu(DOWN);
	if (key == MYLEFT)  action = updateMenu(LEFT);
	if (key == MYRIGHT)  action = updateMenu(RIGHT);
	return action;
}//update------------------------------------------

//update===========================================
//The Sketche's keypad returns chars.
// See Menu::mapKeyChar().
//See Menu::updateMenu() for more details.
//-------------------------------------------------
int Menu::update(char key) {
  if (key == char(0)) return 0;
  int action = 0;
	if (key == MYCHARUP)  action = updateMenu(UP);
	if (key == MYCHARDOWN)  action = updateMenu(DOWN);
	if (key == MYCHARLEFT)  action = updateMenu(LEFT);
	if (key == MYCHARRIGHT)  action = updateMenu(RIGHT);
  return action;
}//update------------------------------------------

