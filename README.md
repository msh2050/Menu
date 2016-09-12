# Menu
A very simple menu library

You create a menu this way :

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



That's it, you're done
