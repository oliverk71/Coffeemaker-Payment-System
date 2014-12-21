Coffeemaker-Payment-System
==========================

An Arduino-based RFID payment system for coffeemakers with toptronic logic unit, as Jura Impressa S95 and others without modifying the coffeemaker itself (to keep the guarantuee). This code works with Jura Impressa S90 and S95 and should work with many others, too, although the commands may have to be modified, because they differ from model to model.

Hardware: RFID reader, HC-05 bluetooth dongle, a buzzer, a16x2 LCD, Arduino Uno, housing, f/m jumper wires and/or sensor cable. Depending on the coffeemaker: D-SUB or 4-pin.
wrote a code and finally got a complete RFID payment system for use in offices and institutes with up to 40 coffee drinkers. The RFID reader could be replaced with a NFC reader, but I had the RFID lying around. 

Advantages: 
Complete low-cost payment system for small offices!
Already existing cards can be used! Any RFID (or NFC) tag can be registered! 
No lose of guarantee! Uses the service port / serial connection of the coffeemaker!
In theory a maximum of 200 cards can be registered, but it is limited to 40 in the code (variable n).
Easy registering, charging, deleting and setting up price list via Android app! Or write your own application to control the device from a Linux or Windows PC.

Coffeemaker communication tool:
This is a kind of translator, which translate every ASCII character entered into the serial console into 4 bytes of machine code. Use this to read the memory of your coffeemaker, switch it on and off, start preparing coffee, for diagnosis and more. But take care not to accidently reset your coffeemaker's memory! A few commands can be found here: http://protocol-jura.do.am/

Coffeemaker PS initialization tool: 
Has to be run once before using the coffeemaker payment system to write zeros to the Arduino's EEPROM, where card numbers, credits and product prices will be stored.If you are not sure: Choose 'i'.

Coffeemaker Payment System v0.6.1: 
This is the actual payment system sketch for Arduino. Pinout see below. You should first write zeros to the EEPROM and activate Inkasso mode by initializing with the above tool. Then upload the program code. Install the app on your phone and start it. Enter prices in the appropriate text boxes, connect to the device and upload the price list including the standard value for newly registered cards. Then start registering your cards. When you are done you should be able to check your credit on your card or draw a coffee.

Coffeemaker PS Android App: You can use this Android app to start registering cards (device will stop registering after 10 s, if no more card is recognized), charge a card with 10 or 20 EUR, delete a card (if it has been lost, for example) and set the price for each product. The app is still a little bit buggy, but you can use any other bluetooth terminal app you want. 
"RRR" triggers registering cards
"CCC031000" charges card on index 03 with 10 EUR.
"REA" to read the price list (not yet working)
"CHA0025,0050,0040,0080,0070,0000,0000,0800" sets the prices entered in the app's textboxes for product 1 to 7 and the value for new cards
"LLL" calls for a list of registered RFID cards

The code is provided 'as is', without any guarantuee. It is still in experimental status and has to be enhanced.

Arduino Pinout:
===============
Digital pin 02 - RFID RX

Digital pin 03 - RFID TX

Digital pin 04 - Bluetooth RX

Digital pin 05 - Bluetooth TX

Digital pin 10 - Coffeemaker RX

Digital pin 11 - Coffeemaker TX

Digital pin 12 - buzzer




Analog pin 04 - LCD I2C SDA

Analog pin 05 - LCD I2C SCL




Jura coffeemakers pinouts
=========================
(taken from http://protocol-jura.do.am/)

Jura 4-pin interface (e.g. Jura Impressa S95):

(from left to right)

pin 4 - +5V

pin 3 - RX

pin 2 - GND

pin 1 - TX



Jura 4-pin interface

(from left to right)

pin 5 - +5V

pin 4 - not used

pin 3 - RX

pin 2 - GND

pin 1 - TX



Jura 7-pin interface

(pin 8 - not used)

pin 7 - not used

pin 6 - +5V

pin 5 - not used

pin 4 - RX

pin 3 - GND

pin 2 - TX

pin 1 - not used

(pin 0 - not used)



Jura 9-pin RS232 interface

pin 1 - TX

pin 2 - out

pin 3 - GND

pin 4 - RX

pin 5 - +5V

pin 6 - not used

pin 7 - not used

pin 8 - not used

pin 9 - not used




