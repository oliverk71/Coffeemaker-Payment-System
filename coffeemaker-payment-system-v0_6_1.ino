
/* An Arduino-based RFID payment system for coffeemakers with toptronic logic unit, as Jura Impressa S95
and others without modifying the coffeemaker itself (to keep the guarantuee). As commands differ from one
coffeemaker to another, they have to be changed in the code, if necessary.

Hardware used: Arduino Uno, 16x2 LCD I2C, "RDM 630" RFID reader 125 kHz, HC-05 bluetooth, buzzer, 
male/female jumper wires, a housing.

I used a RFID reader, but of course it would be possible to use NFC instead. Already existing cards can be used! Any RFID 
(or NFC) tag can be registered. Registering of new cards, charging (up to 25,50 EUR by now, may be changed,
too) and deleting of old cards is triggered via the Android app.´

The code is provided 'as is', without any guarantuee.*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define error(s) error_P(PSTR(s))

SoftwareSerial myBT(4,5); // RX, TX
SoftwareSerial myCoffeemaker(10,11); // RX, TX

int i;
int k; 
const int n = 40;  // max total number of cards with access (up to 200 max!)

// RFID related variables
byte RFIDcardNum[4];
byte evenBit = 0;
byte oddBit = 0;
byte isData0Low = 0;
byte isData1Low = 0;
int recvBitCount = 0;
byte isCardReadOver = 0;
boolean access;
long int RFIDcard = 0;

long int RFIDcards[n] = {
  0,0,0}; 
int creditArray[n] = {};  
  
// EEPROM related variables
byte cardByte[n*6];
long int cardNr;

String BTstring="";

int price;  
int priceArray[8];  // 0 = PAA, 1 = PAB usw

unsigned long time;

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

union{
  byte priceByte[2];
  int priceInt;  
} 
priceConvert;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.print(F("CoffeemakerPS"));
  delay(3500);
  lcd.clear();
  lcd.print(F("starting up"));
  Serial.begin(9600);         // start serial communication at 9600bps
//  myBT.begin(9600);           // Bluetooth at 9600bps (default)
//  myCoffeemaker.begin(9600);  // Coffeemaker serial communication
  attachInterrupt(0, ISRreceiveData0, FALLING );  // RFID: data0/rx is connected to pin 2, which results in INT 0
  attachInterrupt(1, ISRreceiveData1, FALLING );  // RFID: data1/tx is connected to pin 3, which results in INT 1
  for (i = 0; i < n; i++){  // read card numbers and referring credit from EEPROM
    cardConvert.cardByte[0] = EEPROM.read(i*6);
    cardConvert.cardByte[1] = EEPROM.read(i*6+1);
    cardConvert.cardByte[2] = EEPROM.read(i*6+2);
    cardConvert.cardByte[3] = EEPROM.read(i*6+3);
    creditConvert.creditByte[0] = EEPROM.read(i*6+4);
    creditConvert.creditByte[1] = EEPROM.read(i*6+5);
    RFIDcards[i]  = cardConvert.cardNr;  // union to put the four bytes together
    creditArray[i] = creditConvert.creditInt;
    Serial.print(F("Card: ")); 
    Serial.print(print10digits(RFIDcards[i]));  
    Serial.print(F("\tCredit: ")); 
    Serial.println(printCredit(creditArray[i]));
  }
  for (i = 0; i < 8; i++){
    priceConvert.priceByte[0] = EEPROM.read(i*2+1000);
    priceConvert.priceByte[1] = EEPROM.read(i*2+1001);
    priceArray[i] = priceConvert.priceInt;
    Serial.print("Product ");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.println(printCredit(priceArray[i]));
  }
    
  lcd.clear();
  lcd.print(F("finished"));
  delay(300);
  lcd.noBacklight();
  lcd.clear();
}

void loop()
{ 
  // RFID to check credit
  RFIDcard = 0;  
  time = millis(); 
  do {
    RFIDcard = RFID();
    if (RFIDcard > 0){
      k = n;
      Serial.print(F("Card number:\t"));
      Serial.println(print10digits(RFIDcard));
      for(int i=0;i<n;i=i++){         
        if (((RFIDcard) == (RFIDcards[i]))){  
          Serial.print(printCredit(creditArray[i]));
          Serial.println(F(" left"));
          lcd.backlight();
          lcd.setCursor(0, 0);
          lcd.print(print10digits(RFIDcard));
          lcd.setCursor(0, 1);
          lcd.print(printCredit(creditArray[i]));
          lcd.print(F(" left"));      
          delay(2000);
          lcd.noBacklight();
          lcd.clear();
          k = i;
          i = n;          
        }
      }
      if (k == n){ 
        Serial.print(F("card "));
        Serial.print(print10digits(RFIDcard));
        Serial.println(F(" unknown!"));
        k=0; 
        beep(3);
      }   
    }
  } 
  while ( (millis()-time) < 60 ); 



// Check if there is a bluetooth connection and command
BTstring = BT();
  
if (BTstring.length() > 0){
    Serial.print(F("String ")); 
    Serial.println(BTstring);
// BT: Registering new cards
    if( BTstring == "RRR" ){           // starts registering cards and stops if longer than 5 sec no card
      time = millis();
      beep(1);
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print(F("Registering"));
        lcd.setCursor(0,1);
        lcd.print(F("new cards"));
      do {
        RFIDcard = 0; 
      do {
        RFIDcard = RFID();
        if (RFIDcard > 0) break;
      } 
      while ( (millis()-time) < 5000 );
      k = 0;
      for(i=0;i<n;i++){
        if ((RFIDcard == RFIDcards[i]) && (RFIDcard > 0) && (k == 0)){   //  && (RFIDcard>0 (((RFIDcard) == (RFIDcards[i])) || ((card) > 0))
          Serial.print(F("card number "));
          Serial.print(print10digits(RFIDcard));
          Serial.println(F(" already exists!"));
          lcd.clear();
          lcd.print(print10digits(RFIDcard));
          lcd.setCursor(0,1);
          lcd.print(F("already exists!"));         
          i = n;
          k = 1;
          time = millis();           
          beep(3);
        }
      }
      for(i=0;i<n;i++){    
        if(RFIDcards[i] == 0 && k == 0 && RFIDcard > 0){
          RFIDcards[i] = RFIDcard;
          creditArray[i] = priceArray[7]; // each new card gets 10 EUR credit
          cardConvert.cardNr = RFIDcards[i];
          creditConvert.creditInt = creditArray[i];
          Serial.print(F("card number "));
          Serial.print(print10digits(RFIDcard));
          Serial.println(F(" registered!"));
          lcd.clear();
          lcd.print(print10digits(RFIDcard));
          lcd.setCursor(0,1);
          lcd.print(F("registered"));
          EEPROM.write(i*6, cardConvert.cardByte[0]);
          EEPROM.write(i*6+1, cardConvert.cardByte[1]);
          EEPROM.write(i*6+2, cardConvert.cardByte[2]);
          EEPROM.write(i*6+3, cardConvert.cardByte[3]);
          EEPROM.write(i*6+4, creditConvert.creditByte[0]); // 10 EUR standard credit for new cards  
          EEPROM.write(i*6+5, creditConvert.creditByte[1]);
          beep(1);
          i = n;
          k = 2;
          time = millis();          
        }   
      }
    } while ( (millis()-time) < 10000 );
    beep(1);
    beep(2);
    beep(1);    
    lcd.clear();
    lcd.noBacklight();    
    }

// BT: Send RFID card numbers to app    
  if(BTstring =="LLL"){  // 'L' for 'list' sends RFID card numbers to app   
    for(byte i=0;i<n;i++){   
      myBT.print(print10digits(RFIDcards[i]));
      if (i < n-1) myBT.write(',');   
    }
  }
  if(BTstring.startsWith("DDD") == true){
      BTstring.remove(0,3); // removes "DDD" and leaves the index
      int i = BTstring.toInt();
      i--; // list picker index (app) starts at 1, while RFIDcards array starts at 0       
      EEPROM.write(i*6, 0);
      EEPROM.write(i*6+1, 0);
      EEPROM.write(i*6+2, 0);
      EEPROM.write(i*6+3, 0);
      EEPROM.write(i*6+4, 0);
      EEPROM.write(i*6+5, 0);
      Serial.print(F("Card number "));
      Serial.print(print10digits(RFIDcards[i]));
      Serial.print(F(" at index number "));
      Serial.println(i);
      Serial.print(F(" has been deleted!"));
      beep(1);
      lcd.backlight();
      lcd.clear();
      lcd.print(print10digits(RFIDcards[i])); 
      lcd.setCursor(0,1);
      lcd.print(F("deleted!"));
      RFIDcards[i] = 0;
      delay(1000); 
  }
    
// BT: Charge a card    
  if(BTstring.startsWith("CCC") == true){
    // first 2 characters after 'C' is the index of the card
    char a1 = BTstring.charAt(3);  // 3 and 4 => index
    char a2 = BTstring.charAt(4);
    char a3 = BTstring.charAt(5);  // 5 and 6 => value to charge
    char a4 = BTstring.charAt(6);    
    BTstring = String(a1)+String(a2); 
    Serial.println(BTstring);
    byte i = BTstring.toInt();    // index of card
    BTstring = String(a3)+String(a4);
    Serial.println(BTstring); 
    int j = BTstring.toInt();   // value to charge
    j *= 100;
    i--; // list picker index (app) starts at 1, while RFIDcards array starts at 0       
    Serial.println(i);
    Serial.println(j);
    Serial.println(creditArray[i]);
    Serial.println(creditArray[i]+j);
    creditConvert.creditInt = (creditArray[i]+j);   
//    creditConvert.creditInt = creditArray[i];
    
    EEPROM.write(i*6+4, creditConvert.creditByte[0]);
    EEPROM.write(i*6+5, creditConvert.creditByte[1]);
    
    
    Serial.print(F("Card number "));
    Serial.print(print10digits(RFIDcards[i]));
    Serial.print(F(" credit "));
    Serial.print(printCredit(creditArray[i]));  // -j gelöscht
    Serial.print(F(" +"));
    Serial.print(printCredit(j));
    Serial.print(F(" = "));
    Serial.println(printCredit(creditArray[i]+j));
    beep(1);
    lcd.backlight();
    lcd.clear();
    lcd.print(print10digits(RFIDcards[i])); 
    lcd.setCursor(0,1);
    lcd.print(printCredit(creditArray[i]));
    lcd.print(F("+"));
    lcd.print(printCredit(j));
    delay(1000);
    lcd.noBacklight();
    lcd.clear();
    creditArray[i] += j;
  }
// BT: Change prices   
  if(BTstring.startsWith("CHA") == true){
    // first 2 characters after "CHA" is the product number (e.g. "01" for small cup)
    k = 3;
    for (i = 0; i < 8;i++){  
      String tempString = "";
      do {
        tempString += BTstring.charAt(k);
        k++;
      } while (BTstring.charAt(k) != ','); 
      j = tempString.toInt();
      Serial.println(tempString); 
      priceConvert.priceInt = j;
      EEPROM.write(i*2+1000, priceConvert.priceByte[0]);
      EEPROM.write(i*2+1001, priceConvert.priceByte[1]);
      priceArray[i] = j;
      k++;
    }
  }
  
  if(BTstring.startsWith("REA") == true){  
    for (i = 0; i < 8; i++) {
      Serial.print(priceArray[i]);
      Serial.print(",");
      myBT.print(priceArray[i]);
      if (i < 7) myBT.write(',');
    }
    Serial.println();
  }  
  
  if(BTstring == "?M3\r\n"){  
    for (i = 0; i < 1024; i++) {    // write a zero to every single byte of the 1 kb EEPROM 
      EEPROM.write(i, 0);
    }
    toCoffeemaker("?M3\r\n");  // activates incasso mode (= no coffee w/o "ok" from the payment system! May be inactivated by sending "?M3" without quotation marks)
  }

  if(BTstring == "?M1\r\n"){  
    for (i = 0; i < 1024; i++) {    // write a zero to every single byte of the 1 kb EEPROM 
      EEPROM.write(i, 0);
    }
    toCoffeemaker("?M1\r\n");  // activates incasso mode (= no coffee w/o "ok" from the payment system! May be inactivated by sending "?M3" without quotation marks)
  }    
  BTstring = ""; // Reset val that it is only read once  
}



  // Get key pressed on coffeemaker
String message = fromCoffeemaker();   // gets answers from coffeemaker 

  if (message.length() > 0){
 
    if (message.charAt(0) == '?'){        // message command? (start with ?)
      if (message == "?PAE\r\n"){
        price = priceArray[0];        // product 1 (small cup)
        Serial.println();
        Serial.print(F("small cup\t"));
      }     
      if (message == "?PAF\r\n"){
        price = priceArray[1];      // product 2 (2 small cups)
        Serial.println();
        Serial.print(F("2 small cups\t"));
      } 
      if (message == "?PAA\r\n"){         // large cup
        price = priceArray[2];   // product 3 (large cup)
        Serial.println();
        Serial.print(F("large cup\t"));
      } 
      if (message == "?PAB\r\n"){
        price = priceArray[31];     // product 4 (2 large cups)
        Serial.println();
        Serial.print(F("2 large cups\t"));
      } 
      if (message == "?PAJ\r\n"){
        price = priceArray[4];      // product 5 (steam)
        Serial.println();
        Serial.print(F("Dampf 2\t"));
      } 
      if (message == "?PAI\r\n"){
        price = priceArray[5];      // product 6 (steam)
        Serial.println();
        Serial.print(F("Dampf 1\t"));
      }    
      if (message == "?PAG\r\n"){
        price = priceArray[6];      // product 7 (extra large cup)
        Serial.println();
        Serial.print(F("extra large cup\t"));
      }  
      else { 
        Serial.print(message);
      }
      Serial.print("-");
      Serial.println(printCredit(price/10));
  
  
      // RFID Identification      
      time = millis();
      RFIDcard = 0;
      do {
        RFIDcard = RFID();
        if (RFIDcard > 0) break;   
      } 
      while ( (millis()-time) < 5000 );
      Serial.print(F("Card number: \t"));
      Serial.println(print10digits(RFIDcard));
      for(int i=0;i<n;i=i++){         
        if (((RFIDcard) == (RFIDcards[i])) && (RFIDcard>0 )){   // (((RFIDcard) == (RFIDcards[i])) || ((card) > 0))
  //        k = i;
  
          // Enough credit?     
          if ((creditArray[i] - price) > 0){
            creditArray[i] -= price;
            Serial.println(printCredit(creditArray[i]));
            lcd.backlight();
            lcd.setCursor(0, 0);
            lcd.print(RFIDcard);
            lcd.setCursor(0, 1);
            lcd.print(printCredit(creditArray[i]));
            lcd.print(F(" left"));
            creditConvert.creditInt = creditArray[i];
            EEPROM.write(i*6+4, creditConvert.creditByte[0]);
            EEPROM.write(i*6+5, creditConvert.creditByte[1]);
            toCoffeemaker(F("?ok\r\n"));          
            delay(2000);
            lcd.noBacklight();
            lcd.clear();       
          } 
          else { 
            Serial.println(printCredit(creditArray[i]));
            Serial.println(F(" not enough credit"));
            lcd.backlight();
            lcd.setCursor(0, 0);
            lcd.print(print10digits(RFIDcard));
            lcd.setCursor(0, 1);
            lcd.print(printCredit(creditArray[i]));
            beep(3);
            lcd.noBacklight();
            beep(3);
            lcd.backlight();
            beep(3);
            lcd.noBacklight();      
            lcd.clear();    
          }
        } 
        else {
          access = false;
        }
      }
    }
  }
}

String BT(){
  myBT.begin(9600);
  delay(90);  // 100
  while( myBT.available() ){  
    BTstring += String(char(myBT.read()));
      delay(60);
  }
 return(BTstring); 
  }

String fromCoffeemaker(){
  myCoffeemaker.begin(9600);
  delay(10); // testweise
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
  myCoffeemaker.begin(9600);
  delay(10); 
  Serial.println();
  Serial.print(outputString);
  Serial.println(outputString.length());
  for (byte a = 0; a < outputString.length(); a++){
    Serial.print(outputString.charAt(a));
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
  int i = 10-tempString.length();
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
    case 1: 
    tone(12,1500,duration);
    delay(duration);
    break;
    case 2:
    tone(12,1000,duration);
    delay(duration);
    break;
    case 3:
    tone(12,500,duration);
    delay(duration);
    break;
    case 4: 
    for (int a = 0; a < 3; a++){
      for (int i = 2300; i > 600; i-=50){
        Serial.print(i);
          tone(12,i,20);
          delay(18);
      }     
      for (int i = 600; i < 2300; i+=50){
        Serial.print(i);
          tone(12,i,20);
          delay(18);
      }
    }
 
  break;  
  }
}



/* RFID READER */
long int RFID(){    
  // RFID READER
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
  int i = 0;
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











