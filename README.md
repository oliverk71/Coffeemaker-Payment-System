Coffeemaker-Payment-System
==========================

An Arduino-based RFID payment system for coffeemakers with toptronic logic print from Eugster electronics, as Jura Impressa S95 and many others. A modification of the coffeemaker's hardware is NOT necessary. This code should work at least with Jura Impressa S90, S95 and X7 although confirmed only for S95 and should work with many others, too. The commands may have to be modified, because they may differ from model to model.    
   
Hardware: RDM630 RFID reader 125 kHz, HC-05 bluetooth dongle, a buzzer, a 16x2 LCD, Arduino Uno, housing, f/m jumper wires and/or sensor cable. Depending on the coffeemaker: D-SUB or 4-pin.     

This payment system would be ideal in offices and institutes with up to 40 coffee drinkers, although up to 200 are possible. But it is also a nice basis for many other projects around the coffeemaker, for example drawing a coffee via Android app or to read the EEPROM memory of a coffee machine.    

I chose the RFID because I had it lying around otherwise I would have chosen a NFC reader, because the tags are more common.     
   
Advantages:   
- Complete low-cost payment system for use in small offices!    
- Already existing cards can be used! Any RFID tag can be registered!    
- Uses the service port / serial connection of the coffeemaker -> no modification of the coffeemaker's hardware necessary!   
- In theory a maximum of 200 cards can be registered, but it is limited by default to 40 (look for variable n to change number of maximum cards).   
- Easy registering, charging, deleting and setting up price list via Android app! Or write your own application to control the device from a Linux or Windows PC.   
- Added a functionality to trigger the coffeemaker using an Android app. It's more a gag and I added voice control. The app recognizes 'small', 'large' or 'extra large' in a sentence and will prepare a coffee. Do not forget to place the cup!    
   
Coffeemaker-communication-tool.ino:     
Basically translates ASCII character entered into the serial console into 4 bytes of coffeemaker machine code. Use this to read the memory of your coffeemaker, switch it on and off, start preparing coffee, for diagnosis and more. But take care not to accidently reset your coffeemaker's memory! A few commands can be found here: http://protocol-jura.do.am/   
     
EEPROM_tool.ino:       
Has to be used once to initiate the coffeemaker payment system to write zeros to the Arduino's EEPROM, where card numbers, credits and product prices will be stored. But it can be used for other purposes, e. g. reading the EEPROM memory of the coffee machine, where relevant data is stored (number of draws, model type etc). All options by now:
(1) read data in Arduino EEPROM and display HEX values         
(2) read data in Arduino EEPROM and display card numbers, credits and price list.       
(3) erase all data in Arduino EEPROM (overwrite with zeros)         
(4) initialize for CoffeemakerPM (erases EEPROM and activates Inkasso mode.       
(5) deactivate incasso mode.          
(6) read EEPROM of the coffeemaker and display the HEX values.              
   
CoffeemakerPS.ino:    
This is the actual payment system sketch for Arduino. Pinout see below. You should first write zeros to the EEPROM and activate Inkasso mode by initializing with the above tool. Then upload the program code. Install the app on your phone and start it. Enter prices in the appropriate text boxes, connect to the device and upload the price list including the standard value for newly registered cards. Then start registering your cards. When you are done you should be able to check your credit on your card or draw a coffee.    
   
CoffeemakerPS.apk: You can use this Android app to start registering cards (device will stop registering after 10 s, if no more card is recognized), charge a card with 10 or 20 EUR, delete a card (if it has been lost, for example) and set the price for each product. The app is still a little bit buggy, but you can use any other bluetooth terminal app you want.    
"RRR" triggers registering cards   
"CCC031000" charges card on index 03 with 10 EUR.   
"REA" to read the price list (not yet working)   
"CHA0025,0050,0040,0080,0070,0000,0000,0800" sets the prices entered in the app's textboxes for product 1 to 7 and the value for new cards   
"LLL" calls for a list of registered RFID cards   

Coffeemaker_voice_control.apk: This is more a gag than a real useful application. The app allows drawing a coffee using voice control or button press. The voice control of the smartphone recognizes the words 'small', 'large' or 'extra large' in a sentence. 
   
The code is provided 'as is', without any guarantuee. It is still in experimental status and has to be enhanced.   
   
Arduino Pinout:   
===============   
Digital pin 00 - RX (bluetooth)   (bt module needs to be disconnected to upload a new sketch or to use EEPROM tool!)      
Digital pin 01 - TX (bluetooth)   
Digital pin 02 - RX (RFID)   
Digital pin 03 - TX (RFID)   
Digital pin 04 - RX (coffeemaker)   
Digital pin 05 - TX (coffeemaker)      
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

Hyperlinks
==========
08.01.2016 (DD.MM.YYYY) Article in German computer magazine c't (German language): http://www.heise.de/ct/ausgabe/2016-2-NFC-Bezahlsystem-fuer-Kaffeevollautomaten-3057531.html?wt_mc=print.ct.2016.02.110#zsdb-article-links
08.01.2016 Sharespresso project website (Adaption of this project): http://www.heise.de/ct/artikel/Sharespresso-NFC-Bezahlsystem-fuer-Kaffeevollautomaten-3058350.html
08.01.2016 Sharespresso on GitHub: https://github.com/psct/sharespresso
27.12.2014 Article on Hackaday: http://hackaday.com/2014/12/27/diy-coffee-payment-system-doesnt-void-your-warranty/
24.11.2014 Previous project not using service-port: https://www.youtube.com/watch?v=hGvdymS_08c
