Coffeemaker-Payment-System
==========================

An Arduino-based RFID payment system for coffeemakers with toptronic logic unit, as Jura Impressa S95 and others without modifying the coffeemaker itself (to keep the guarantuee). 
As commands differ from one coffeemaker to another, they have to be changed in the code, if necessary. 

I connected a RFID reader, a HC-05 bluetooth dongle, a buzzer and a 16x2 LCD to an Arduino Uno and put it into a housing, wrote a code and finally got a complete RFID payment system for use in offices and institutes with up to 40 coffee drinkers. The RFID reader can be replaced easily with a NFC reader. Already existing cards can be used! Any RFID (or NFC) tag can be registered. 

Arduino Pinout:
===============
Digital pin 02 - RFID RX\n
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

