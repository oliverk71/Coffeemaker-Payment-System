
/* An Arduino-based RFID payment system for coffeemakers with toptronic logic unit, as Jura Impressa S95
 and others without modifying the coffeemaker itself. Commands may differ from one coffeemaker to another, 
 they have to be changed in the code, if necessary. Make sure not to accidently reset your coffeemaker's 
 EEPROM! Make a backup, if necessary. 
 
 Hardware used: Arduino Uno, 16x2 LCD I2C, "RDM 630" RFID reader 125 kHz, HC-05 bluetooth, buzzer, 
 male/female jumper wires, a housing.
 
 pinouts:
 Analog pins 4,5 - LCD I2C
 Digital pins 0,1 - myBT bluetooth RX, TX (hardware serial)
 Digital pins 2,3 - RFID RX, TX
 Digital pins 4,5 - myCoffeemaker RX, TX (software serial)
 Digital pin 12 - piezo buzzer
 
 Already existing cards can be used! Any 125 kHz RFID tag can be registered. Registering of new cards, 
 charging and deleting of old cards is done via the Android app, but it would be possible to use any 
 other bluetooth client software on smartphone or PC.
 
 The code is provided 'as is', without any guarantuee. Use at your own risk! */

#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define error(s) error_P(PSTR(s))

LiquidCrystal_I2C lcd(0x27,16,2);
SoftwareSerial myCoffeemaker(4,5); // RX, TX

// general variables
int i;
int j;
int k; 
boolean buttonPress = false;
const int n = 20;  // max total number of cards with access (up to 200 cards max = 199! Do not exceed otherwise you will overwrite price list!)
int creditArray[n] = {      // remaining credit on card
}; 
int price;                  // price variable
int priceArray[11];  // price Array contains prices for up to 10 products. 0 = PAA, 1 = PAB usw. Plus standard value for new cards
String BTstring="";
unsigned long time;
unsigned long buttonTime;

// RFID related variables
byte cardByte[4];
long int cardNr;
byte RFIDcardNum[4];
byte evenBit = 0;
byte oddBit = 0;
byte isData0Low = 0;
byte isData1Low = 0;
int recvBitCount = 0;
byte isCardReadOver = 0;
long int RFIDcard = 0;
long int RFIDcards[n] = {
}; 

union{
  byte cardByte[4];
  long int cardNr;
} 
cardConvert;

union{
  byte creditByte[2];
  int creditInt;
} 
creditConvert;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.print(F("CoffeemakerPS"));
  lcd.setCursor(0,1);
  lcd.print(F("  v 0.7"));
  delay(3500);
  lcd.clear();
  lcd.print(F("starting up"));
  myCoffeemaker.begin(9600);         // start serial communication at 9600bps
  Serial.begin(9600);           // Bluetooth at 9600bps (default)
  attachInterrupt(0, ISRreceiveData0, FALLING );  // RFID: data0/rx is connected to pin 2, which results in INT 0
  attachInterrupt(1, ISRreceiveData1, FALLING );  // RFID: data1/tx is connected to pin 3, which results in INT 1
  for (int i = 0; i < n; i++){  // read card numbers and referring credit from EEPROM
    cardConvert.cardByte[0] = EEPROM.read(i*6);
    cardConvert.cardByte[1] = EEPROM.read(i*6+1);
    cardConvert.cardByte[2] = EEPROM.read(i*6+2);
    cardConvert.cardByte[3] = EEPROM.read(i*6+3);
    RFIDcards[i]  = cardConvert.cardNr;  // union to put the four bytes together
    creditConvert.creditByte[0] = EEPROM.read(i*6+4);
    creditConvert.creditByte[1] = EEPROM.read(i*6+5);
    creditArray[i] = creditConvert.creditInt;
  }
  for (int i = 0; i < 11; i++){   // read price list products 1 to 10 and start value for new cards
    creditConvert.creditByte[0] = EEPROM.read(i*2+1000);
    creditConvert.creditByte[1] = EEPROM.read(i*2+1001);
    priceArray[i] = creditConvert.creditInt;
  }

  lcd.clear();
  lcd.print(F("finished"));
  delay(300);
  lcd.noBacklight();
  lcd.clear();
}

void loop()
{ 
  // Check if there is a bluetooth connection and command
  BTstring = "";
  //  buttonPress = false;
  while( Serial.available() ){  
    BTstring += String(char(Serial.read()));
    delay(7);  // 60 hatte funktioniert angeblich, 1 nicht, mal 7 testen. auch nicht
  }
  if (BTstring.length() > 0){
    // BT: Start registering new cards until 10 s no valid, unregistered card
    if( BTstring == "RRR" ){          
      time = millis();
      beep(1);
      lcd.backlight();
      lcd.print(F("Registering"));
      lcd.setCursor(0,1);
      lcd.print(F("new cards"));
      do {
        RFIDcard = 0; 
        do {
          RFIDcard = RFID();
          if (RFIDcard > 0) break;
        } 
        while ( (millis()-time) < 60 );   // changed to 60 (was 5000)
        k = 0;
        for(i=0;i<n;i++){
          if ((RFIDcard == RFIDcards[i]) && (RFIDcard > 0) && (k == 0)){   //  && (RFIDcard>0 (((RFIDcard) == (RFIDcards[i])) || ((card) > 0))
            lcd.clear();
            lcd.print(print10digits(RFIDcard));
            lcd.setCursor(0,1);
            lcd.print(F("already exists!"));         
            i = n;
            k = 1;
            time = millis();           
            beep(2);
          }
        }
        for(i=0;i<n;i++){    
          if(RFIDcards[i] == 0 && k == 0 && RFIDcard > 0){
            RFIDcards[i] = RFIDcard;
            lcd.clear();
            lcd.print(print10digits(RFIDcard));
            lcd.setCursor(0,1);
            lcd.print(F("registered"));
            cardConvert.cardNr = RFIDcard;
            EEPROM.write(i*6, cardConvert.cardByte[0]);
            EEPROM.write(i*6+1, cardConvert.cardByte[1]);
            EEPROM.write(i*6+2, cardConvert.cardByte[2]);
            EEPROM.write(i*6+3, cardConvert.cardByte[3]);
            creditArray[i] = priceArray[10]; // standard credit for newly registered cards          
            creditConvert.creditInt = creditArray[i];
            EEPROM.write(i*6+4, creditConvert.creditByte[0]); // 10 EUR standard credit for new cards  
            EEPROM.write(i*6+5, creditConvert.creditByte[1]);
            beep(1);
            i = n;
            k = 2;
            time = millis();          
          }   
        }
      } 
      while ( (millis()-time) < 10000 );
      beep(3);   
      lcd.clear();
      lcd.noBacklight();    
    }
    // BT: Send RFID card numbers to app    
    if(BTstring == "LLL"){  // 'L' for 'list' sends RFID card numbers to app   
      for(byte i=0;i<n;i++){  // variable type has to be byte otherwise it does not work
        Serial.print(print10digits(RFIDcards[i])); 
        if (i < (n-1)) Serial.write(',');  // write comma after card number if not last
      }
    }
    // BT: Delete a card and referring credit   
    if(BTstring.startsWith("DDD") == true){
      BTstring.remove(0,3); // removes "DDD" and leaves the index
      i = BTstring.toInt();
      i--; // list picker index (app) starts at 1, while RFIDcards array starts at 0       
      EEPROM.write(i*6, 0);    // writes card number (4 bytes)
      EEPROM.write(i*6+1, 0);
      EEPROM.write(i*6+2, 0);
      EEPROM.write(i*6+3, 0);
      EEPROM.write(i*6+4, 0);  // writes credit (2 bytes)
      EEPROM.write(i*6+5, 0);
      beep(1);
      lcd.backlight();
      lcd.print(print10digits(RFIDcards[i])); 
      lcd.setCursor(0,1);
      lcd.print(F("deleted!"));
      RFIDcards[i] = 0;
      delay(2000);
      lcd.clear();
      lcd.noBacklight(); 
    }    
    // BT: Charge a card    
    if((BTstring.startsWith("CCC") == true) ){  // && (BTstring.length() >= 7 )
      char a1 = BTstring.charAt(3);  // 3 and 4 => card list picker index (from app)
      char a2 = BTstring.charAt(4);
      char a3 = BTstring.charAt(5);  // 5 and 6 => value to charge
      char a4 = BTstring.charAt(6);    
      BTstring = String(a1)+String(a2); 
      i = BTstring.toInt();    // index of card
      BTstring = String(a3)+String(a4);
      j = BTstring.toInt();   // value to charge
      j *= 100;
      i--; // list picker index (app) starts at 1, while RFIDcards array starts at 0  
      creditArray[i] += j;
      creditConvert.creditInt = creditArray[i];     
      EEPROM.write(i*6+4, creditConvert.creditByte[0]);
      EEPROM.write(i*6+5, creditConvert.creditByte[1]); 
      beep(1);
      lcd.backlight();
      lcd.print(print10digits((RFIDcards[i])-j)); 
      lcd.setCursor(0,1);
      lcd.print(F("+"));   
      lcd.print(printCredit(j));
      delay(2000);
      lcd.noBacklight();
      lcd.clear();
    }
    // BT: Receives (updated) price list from app.  
    if(BTstring.startsWith("CHA") == true){
      k = 3;
      for (i = 0; i < 11;i++){  
        String tempString = "";
        do {
          tempString += BTstring.charAt(k);
          k++;
        } 
        while (BTstring.charAt(k) != ','); 
        j = tempString.toInt();
        priceArray[i] = j;
        creditConvert.creditInt = j;
        EEPROM.write(i*2+1000, creditConvert.creditByte[0]);
        EEPROM.write(i*2+1001, creditConvert.creditByte[1]);
        k++;
      }
      beep(1);
      lcd.backlight();
      lcd.print("Pricelist");
      lcd.setCursor(0,1);
      lcd.print("updated!");
      delay(2000);
      lcd.noBacklight();
      lcd.clear();
    }
    // BT: Sends price list to app. Product 1 to 10 (0-9), prices divided by commas plus standard value for new cards
    if(BTstring.startsWith("REA") == true){
     // delay(100); // testweise      
      for (i = 0; i < 11; i++) {
        Serial.print(int(priceArray[i]/100));
        Serial.print('.');
        if ((priceArray[i]%100) < 10){
          Serial.print('0');
        }
        Serial.print(priceArray[i]%100);
        if (i < 10) Serial.write(',');
      }
    }  

    if(BTstring == "?M3"){  
      toCoffeemaker("?M3\r\n");  // activates incasso mode (= no coffee w/o "ok" from the payment system! May be inactivated by sending "?M3" without quotation marks)
      delay (100);               // wait for answer from coffeemaker
      lcd.backlight();
      if (fromCoffeemaker() == "?ok\r\n"){
        beep(1);
        lcd.print(F("Inkasso mode"));
        lcd.setCursor(0,1);
        lcd.print(F("activated!"));  
      } 
      else {
        beep(2);
        lcd.print(F("Coffeemaker"));
        lcd.setCursor(0,1);
        lcd.print(F("not responding!"));  
      }
      delay(2000);
      lcd.clear();
      lcd.noBacklight();     
    }

    if(BTstring == "?M1"){  
      toCoffeemaker("?M1\r\n");  // deactivates incasso mode (= no coffee w/o "ok" from the payment system! May be inactivated by sending "?M3" without quotation marks)
      delay (100);               // wait for answer from coffeemaker
      lcd.backlight();
      if (fromCoffeemaker() == "?ok\r\n"){
        beep(1);
        lcd.print(F("Inkasso mode"));
        lcd.setCursor(0,1);
        lcd.print(F("deactivated!"));  
      } 
      else {
        beep(2);
        lcd.print(F("Coffeemaker"));
        lcd.setCursor(0,1);
        lcd.print(F("not responding!"));  
      }
      delay(2000);
      lcd.clear();
      lcd.noBacklight();     
    }    
  }

  // Get key pressed on coffeemaker
  String message = fromCoffeemaker();   // gets answers from coffeemaker 
  if (message.length() > 0){
    if (message.charAt(0) == '?' && message.charAt(1) == 'P'){     // message starts with '?P' ?
      buttonPress = true;
      buttonTime = millis(); 
      lcd.backlight();
      if (message == "?PAE\r\n"){
        price = priceArray[0];        // product 1 (small cup)
        lcd.print(F("Small cup"));
      }     
      else if (message == "?PAF\r\n"){
        price = priceArray[1];      // product 2 (2 small cups)
        lcd.print(F("2 small cups"));
      } 
      else if (message == "?PAA\r\n"){         
        price = priceArray[2];   // product 3 (large cup)
        lcd.print(F("Large cup"));
      } 
      else if (message == "?PAB\r\n"){
        price = priceArray[3];     // product 4 (2 large cups)
        lcd.print(F("2 large cups"));
      } 
      else if (message == "?PAJ\r\n"){
        price = priceArray[4];      // product 5 (steam)
        lcd.print(F("Steam 2"));
      } 
      else if (message == "?PAI\r\n"){
        price = priceArray[5];      // product 6 (steam)
        lcd.print(F("Steam 1"));
      }    
      else if (message == "?PAG\r\n"){
        price = priceArray[6];      // product 7 (extra large cup)
        lcd.print(F("Extra large cup"));
      } 
      else {
        lcd.print(F("Read error"));
        buttonPress = false;
      }
      lcd.setCursor(0,1);  
      lcd.print(printCredit(price));
    }
  }

  if (millis()-buttonTime > 5000){  
    buttonPress = false;
    price = 0;
    lcd.noBacklight();
    lcd.clear();     
  }

  // RFID Identification      
  RFIDcard = 0;  
  time = millis(); 
  do {
    RFIDcard = RFID();
    if (RFIDcard > 0) {
      lcd.clear();
      break; 
    }           
  } 
  while ( (millis()-time) < 60 );  

  if (RFIDcard > 0){
    k = n;
    for(byte i=0;i<n;i=i++){         
      if (((RFIDcard) == (RFIDcards[i])) && (RFIDcard > 0 )){
        k = i;
        if(buttonPress == true){                 // button pressed on coffeemaker?
          if ((creditArray[k] - price) > 0){     // enough credit?
            creditArray[k] -= price;
            lcd.backlight();
            lcd.setCursor(0, 0);
            lcd.print(print10digits(RFIDcards[k]));
            lcd.setCursor(0, 1);
            lcd.print(printCredit(creditArray[k]));
            creditConvert.creditInt = creditArray[k];
            EEPROM.write(k*6+4, creditConvert.creditByte[0]);
            EEPROM.write(k*6+5, creditConvert.creditByte[1]);
            toCoffeemaker("?ok\r\n");            // prepare coffee
          } 
          else {                                 // not enough credit!
            lcd.backlight();
            lcd.setCursor(0, 0);
            lcd.print(print10digits(RFIDcard));
            lcd.setCursor(0, 1);
            lcd.print(printCredit(creditArray[k]));
            beep(2);  
          }
        } 
        else {                                // if no button was pressed on coffeemaker / check credit
          lcd.backlight();
          lcd.print(print10digits(RFIDcards[k]));
          lcd.setCursor(0, 1);
          lcd.print(printCredit(creditArray[k]));      
        }
        delay(2000);
        lcd.noBacklight();
        lcd.clear();      
        i = n;      // leave loop (after card has been identified)
      }      
    }
    if (k == n){ 
      lcd.backlight();
      lcd.print(print10digits(RFIDcard));
      lcd.setCursor(0,1);
      lcd.print(F("card unknown!"));		
      k=0; 
      beep(2);
    }     	    
  }
}

String fromCoffeemaker(){
  String inputString = "";
  char d4 = 255;
  while (myCoffeemaker.available()){    // if data is available to read
    byte d0 = myCoffeemaker.read();
    delay (1); 
    byte d1 = myCoffeemaker.read();
    delay (1); 
    byte d2 = myCoffeemaker.read();
    delay (1); 
    byte d3 = myCoffeemaker.read();
    delay (7);
    bitWrite(d4, 0, bitRead(d0,2));
    bitWrite(d4, 1, bitRead(d0,5));
    bitWrite(d4, 2, bitRead(d1,2));
    bitWrite(d4, 3, bitRead(d1,5));
    bitWrite(d4, 4, bitRead(d2,2));
    bitWrite(d4, 5, bitRead(d2,5));
    bitWrite(d4, 6, bitRead(d3,2));
    bitWrite(d4, 7, bitRead(d3,5));
    if (d4 != 10){ 
      inputString += d4;
    } 
    else { 
      inputString += d4;
      return(inputString);
    } 
  } 
}

void toCoffeemaker(String outputString)
{
  for (byte a = 0; a < outputString.length(); a++){
    byte d0 = 255;
    byte d1 = 255;
    byte d2 = 255;
    byte d3 = 255;
    bitWrite(d0, 2, bitRead(outputString.charAt(a),0));
    bitWrite(d0, 5, bitRead(outputString.charAt(a),1));
    bitWrite(d1, 2, bitRead(outputString.charAt(a),2));  
    bitWrite(d1, 5, bitRead(outputString.charAt(a),3));
    bitWrite(d2, 2, bitRead(outputString.charAt(a),4));
    bitWrite(d2, 5, bitRead(outputString.charAt(a),5));
    bitWrite(d3, 2, bitRead(outputString.charAt(a),6));  
    bitWrite(d3, 5, bitRead(outputString.charAt(a),7)); 
    myCoffeemaker.write(d0); 
    delay(1);
    myCoffeemaker.write(d1); 
    delay(1);
    myCoffeemaker.write(d2); 
    delay(1);
    myCoffeemaker.write(d3); 
    delay(7);
  }
}

String printCredit(int credit){
  int euro = ((credit)/100);  //  int euro = ((credit*10)/100);
  int cent = ((credit)%100);  //  int cent = ((credit*10)%100); 
  String(output);
  output = String(euro);
  output += ',';
  output += String(cent);
  if (cent < 10){
    output += '0';
  }
  output += F(" EUR");  
  return output;
}

String print10digits(long int number) {
  String(tempString) = String(number);
  String(newString) = "";
  i = 10-tempString.length();
  for (int a = 0; a < (10-tempString.length()); a++){
    newString += "0";
  }
  newString += number;
  return newString;
}

String print2digits(int number) {
  String partString;
  if (number >= 0 && number < 10) {
    partString = "0";
    partString += number;
  } 
  else partString = String(number);
  return partString;
}

void beep(byte number){
  int duration = 200;
  switch (number) {
  case 1: // positive feedback
    tone(12,1500,duration);
    delay(duration);
    break;
  case 2: // negative feedback
    tone(12,500,duration);
    delay(duration);
    break;     
  case 3:  // action stopped (e.g. registering) double beep
    tone(12,1000,duration);
    delay(duration);
    tone(12,1500,duration);
    delay(duration);    
    break; 
  case 4:  // alarm (for whatever)
    for (int a = 0; a < 3; a++){
      for (i = 2300; i > 600; i-=50){
        tone(12,i,20);
        delay(18);
      }     
      for (i = 600; i < 2300; i+=50){
        tone(12,i,20);
        delay(18);
      }
    }  
  }
}

/* RFID READER */
long int RFID(){    
  //read card number bit
  if(isData0Low||isData1Low){
    if(1 == recvBitCount){//even bit
      evenBit = (1-isData0Low)&isData1Low;
    }
    else if( recvBitCount >= 26){//odd bit
      oddBit = (1-isData0Low)&isData1Low;
      isCardReadOver = 1;
      delay(10);   // test
    }
    else{
      //only if isData1Low = 1, card bit could be 1
      RFIDcardNum[2-(recvBitCount-2)/8] |= (isData1Low << (7-(recvBitCount-2)%8));
    }
    isData0Low = 0;
    isData1Low = 0;
  }
  if(isCardReadOver){
    if(checkParity()){
      RFIDcard = (*((long *)RFIDcardNum));
      beep(1);
    }
    resetData(); 
  }
  return (RFIDcard);
}

byte checkParity(){
  i = 0;
  int evenCount = 0;
  int oddCount = 0;
  for(i = 0; i < 8; i++){
    if(RFIDcardNum[2]&(0x80>>i)){
      evenCount++;
    }
  }
  for(i = 0; i < 4; i++){
    if(RFIDcardNum[1]&(0x80>>i)){
      evenCount++;
    }
  }
  for(i = 4; i < 8; i++){
    if(RFIDcardNum[1]&(0x80>>i)){
      oddCount++;
    }
  }
  for(i = 0; i < 8; i++){
    if(RFIDcardNum[0]&(0x80>>i)){
      oddCount++;
    }
  }
  if(evenCount%2 == evenBit && oddCount%2 != oddBit){
    return 1;
  }
  else{
    return 0;
  }
}

void resetData(){
  RFIDcardNum[0] = 0;
  RFIDcardNum[1] = 0;
  RFIDcardNum[2] = 0;
  RFIDcardNum[3] = 0;
  evenBit = 0;
  oddBit = 0;
  recvBitCount = 0;
  isData0Low = 0;
  isData1Low = 0;
  isCardReadOver = 0;
}
// handle interrupt0
void ISRreceiveData0(){
  recvBitCount++;
  isData0Low = 1;
}

// handle interrupt1
void ISRreceiveData1(){
  recvBitCount++;
  isData1Low = 1;
}















