
/*
 * Simple_menu
 * Author : Jacques Bellavance
 * Date : September 12, 2016
 * Version 1.00
 */
#include <Menu.h>

/////////////////////////////////////////////////////////////////////////////////////////////THE LCD
//Use your favourite LCD=======================================================
//Standard LCD with Adafruit i2c/SPI backpack.
//Adafruit MCP23008 based backpack I2C interface.
//See https://www.adafruit.com/products/292 for more on this setup.
//-----------------------------------------------------------------------------
#include <Wire.h>
#include <LiquidTWI.h>
LiquidTWI lcd(0);      //Or the I2C address you have setup your backpack
int lcdNumCols = 20;
int lcdNumLines = 4;
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////THE SWITCHES
//Use your favourite keypad============================================================
//For this example, let's use '2' as UP, '8' as DOWN, '4' as LEFT and '6' as RIGHT
//If your keypad returns integers, UP = 1, DOWN = 2, LEFT =3 and RIGHT = 4. (None = 0)
//To read this particular keypad, use keypad.getKey().
//See http://playground.arduino.cc/Code/Keypad for more informations about this keypad
//-------------------------------------------------------------------------------------
#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
   {'1','2','3'},
   {'4','5','6'},
   {'7','8','9'},
   {'#','0','*'}
};
 byte rowPins[ROWS] = {46, 47, 48, 49}; //connect to the row pinouts of the keypad
 byte colPins[COLS] = {50, 51, 52};     //connect to the column pinouts of the keypad

 Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
//------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////THE MENU
//===========================================================================
//Build your menu. On each line, between quotes, lies an item of your menu :
//- Dashes to specify the level of the item,
//- A label to be displayed on the LCD,
//- A colon ":",
//- A number between "000" and "999" to identify an action to be performed,
//  (It has to be exactly 3 characters long and only be digits.)
//  (use "000" to say : "I have a submenu.")
//Don't forget to place a semi-colon ";" at the end of the last line.
//That's it, you're done!
//---------------------------------------------------------------------------
const String menuItems = 
"-READ:000"
"--SENSORS:000"
"---SENSOR A1:101"
"---SENSOR A2:102"
"--SWITCHES:000"
"---SWITCH PIN 4:103"
"---SWITCH PIN 5:104"
"-SET:000"
"--SERVO ARM:105"
"--SERVO BASE:106"
"-MOVE SERVOS:107"
"-LONG ROUTINE:108";

Menu menu(menuItems); //Set up menu
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////THE SKETCH
#define UP '2'             //Use the values for the keys on your keypad.
#define DOWN '8'
#define LEFT '4'
#define RIGHT '6'

#define ANALOG 1          //For the four actions in the menu READ ("101" to "104").
#define DIGITAL 0

#include <Servo.h>        //To set and move the servos ("105" to "107").
int pinServoArm = 2;      //PWM pins only.
int pinServoBase = 3;
int angleServoArm = 90;
int angleServoBase = 90;
Servo servoArm;
Servo servoBase;

//make==========================================================================================
//This is where the action is.
//For each item in the menu that has to do an action,
//We use "switch" to select the action matching the value of the item in the menu. (101 to 107)
//----------------------------------------------------------------------------------------------
void make(int action) {
  switch (action) {
    case 101: { readPin(A1, ANALOG); break; }
    case 102: { readPin(A2, ANALOG); break; }
    case 103: { readPin(4, DIGITAL); menu.reset(); break; }
    case 104: { readPin(5, DIGITAL); menu.reset(); break; }
    case 105: { angleServoArm = changeValue(angleServoArm); break; }
    case 106: { angleServoBase = changeValue(angleServoBase); break; }
    case 107: { servoArm.write(angleServoArm); servoBase.write(angleServoBase); break; }
    case 108: { bool ErrorFlag = longRoutine(); break; }  //You can act on ErrorFlag before break
  }
}//make------------------------------------------------------------------------------------------

//showMenu==============================================================
//Sends the current menu or submenu to your LCD.
//Updated only if needed.
//----------------------------------------------------------------------
void showMenu() {
  if (menu.LcdNeedsUpdate()) {
    lcd.clear();
    for (int i = 0 ; i < lcdNumLines ; i++) { 
      lcd.setCursor(0,i);
      lcd.print(menu.lcdLine(i)); 
    }
    delay(100);
    menu.LcdUpdated();
  }
}//showMenu-------------------------------------------------------------

//setup============================================================================================
void setup() {
  Serial.begin(9600);                                  //Debug only (not needed).
  lcd.begin(lcdNumCols, lcdNumLines);                  //Setup your LCD.
  menu.defineLcd(lcdNumCols, lcdNumLines);             //Describe your LCD to the Menu Library.
  menu.mapKeyChar(UP, DOWN, LEFT, RIGHT);              //Tell the Menu Library what your keys are.

//Your setup here
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  servoArm.attach(pinServoArm);
  servoBase.attach(pinServoBase);
}//setup-------------------------------------------------------------------------------------------

//loop======================================================================================
void loop() {
  showMenu();                      //Update the LCD with the menu.
  char key = keypad.getKey();      //Read the key.
  int action = menu.update(key);   //Read the action tagged to the current menu item.
  if (action > 0) {                //If we need to act:
    make(action);                    //make it.
    menu.done();                     //We are done with this action... Go back to the menu.
  }
}//loop-------------------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////////////////////////THE SKETCH'S FUNCTIONS
//readPin================================================================================
//Used by the four menu items under READ in our menu (actions "101" to "104")
//Prints the label of the current menu item on line 0 of the LCD
//Prints the actual value of the associated pin on line 1 of the LCD
//pinType is ANALOG or DIGITAL
//LEFT or RIGHT terminates the action
//---------------------------------------------------------------------------------------
void readPin(int pin, int pinType) {
  char key = '0';
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print(menu.getCurrentLabel());                    //Print the menu label.
  while(key != LEFT && key != RIGHT) {                  //Exit on LEFT or RIGHT.
    lcd.setCursor(0, 1);
    lcd.print("    ");                                    //Erase the old value.
    lcd.setCursor(0, 1);
    if (pinType == DIGITAL)                             //If DIGITAL:
      lcd.print(digitalRead(pin) ? "HIGH" : "LOW");      //Print HIGH or LOW.
    else                                                //If not,
      lcd.print(analogRead(pin));                         //Print value (0:1023).
    key = keypad.getKey();                              //Read the keypad.
    delay(100);                                         //Reduce flickering.
  }
}//readPin------------------------------------------------------------------------------

//changeValue==========================================================================================
//For the two menu items under SET (actions "105" to "106").
//Prints the label of the current menu item on line 0 of the LCD.
//Prints the actual angle of the servo on row 1 of LCD
//LEFT returns the original value.
//RIGHT returns the modified value.
//UP increases the value by 1, up to 180 degrees.
//DOWN decreases the value by 1, down to 0 degrees.
//-----------------------------------------------------------------------------------------------------
int changeValue(int val) {
  bool changed = true;               //Raise the redraw flag.
  int originalValue = val;           //Remember the original value. 
  char key = '0';                    //Read the keypad to exit.
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(menu.getCurrentLabel()); //Print the menu label
  while (true) {                                                     //Exits when key is LEFT or RIGHT.
    key = keypad.getKey();                                             //Read the keypad.
    if (key == RIGHT) return val;                                      //Exit with new value.
    if (key == LEFT) return originalValue;                             //Exit with original value.
    if (key == UP && val < 180 ) { val++; changed = true; }            //Increase value.
    if (key == DOWN && val > 0) { val--; changed = true; }             //Decrease value.
    if (changed) {                                                     //Redraw ?
      lcd.setCursor(0, 1); lcd.print("   ");                             //Erase old value. 
      lcd.setCursor(0, 1); lcd.print(val);                               //Print the new value.
      changed = false;                                                   //Lower the Redraw flag.
    }
  }
}//changeValue-----------------------------------------------------------------------------------------

//longRoutine==========================================================
//Exiting from a long routine before it's finished.
//This action takes around 20 minutes to acomplish.
//In order to let the servos reach destination, we normaly use delays.
//Replace delays with reading the keypad.
//This keypad reads around 100000 times per second.
//Exit if the keypad returns RIGHT at any point.
//---------------------------------------------------------------------
bool longRoutine() {
  int servoA[4] = {30, 0, 0, 30};
  int servoB[4] = {0, 0, 180, 180};
  long int timing[4] = {10000, 50000, 10000, 50000};

  lcd.clear(); lcd.setCursor(0, 0); lcd.print(">EXIT");

  for (int loops = 1 ; loops < 1000 ; loops++) {
    for (int i = 0 ; i < 4 ; i++) {
      servoArm.write(servoA[i]);
      servoBase.write(servoB[i]);
      for (long int timer = 1; timer <= timing[i]; timer++) {
        if (keypad.getKey() == RIGHT) return true;
      }
    }
  }
  return false;
}//longRoutine---------------------------------------------------------


