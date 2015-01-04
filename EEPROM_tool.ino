#include <SoftwareSerial.h>
#include <EEPROM.h>

SoftwareSerial myCoffeemaker(4,5); // RX, TX

long int cardNr;


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

void setup() {
  Serial.begin(9600);
  myCoffeemaker.begin(9600);
  Serial.println("(1) read data in Arduino EEPROM and display HEX values ");
  Serial.println("(2) read data in Arduino EEPROM and display card numbers, credits and price list.");
  Serial.println("(3) erase all data in Arduino EEPROM (overwrite with zeros)");
  Serial.println("(4) initialize for CoffeemakerPM (erases EEPROM and activates Inkasso mode.)");
  Serial.println("(5) deactivate incasso mode.");
  Serial.println("(6) read EEPROM of the coffeemaker and display the HEX values.");
}

void loop() {
  if (Serial.available()){
    char c = Serial.read();
    if (c == '1'){
      for (int a = 0; a < 1024; a+=16){
        for (int a2 = 0; a2 < 16; a2++){
          byte b = EEPROM.read(a+a2);
          if (b < 16){
            Serial.print('0');
          }
          Serial.print(b, HEX);
          Serial.print("\t");
        }
        Serial.println();
      }  
    }

    if (c == '2'){
        Serial.println();
        Serial.println("Registered cards");
        Serial.println("================");      
      for (int i = 0; i < 40; i++){  // read card numbers and referring credit from EEPROM
        cardConvert.cardByte[0] = EEPROM.read(i*6);
        cardConvert.cardByte[1] = EEPROM.read(i*6+1);
        cardConvert.cardByte[2] = EEPROM.read(i*6+2);
        cardConvert.cardByte[3] = EEPROM.read(i*6+3);
        Serial.print(i);
        Serial.print("\t");
        Serial.print(print10digits(cardConvert.cardNr));
        Serial.print("\t");
        creditConvert.creditByte[0] = EEPROM.read(i*6+4);
        creditConvert.creditByte[1] = EEPROM.read(i*6+5);
        Serial.println(printCredit(creditConvert.creditInt));
      }
      Serial.println();
      Serial.println("Price list");
      Serial.println("==========");
      for (int i = 0; i < 10; i++){   // read price list products 1 to 10 and start value for new cards
        creditConvert.creditByte[0] = EEPROM.read(i*2+1000);
        creditConvert.creditByte[1] = EEPROM.read(i*2+1001);
        Serial.print("product ");
        Serial.print(i+1);
        Serial.print("\t");
        Serial.println(printCredit(creditConvert.creditInt));
      }     
      Serial.println();
      Serial.print("Standard value for newly registered cards: ");
      creditConvert.creditByte[0] = EEPROM.read(10*2+1000);
      creditConvert.creditByte[1] = EEPROM.read(10*2+1001);    
      Serial.println(printCredit(creditConvert.creditInt));  
    }
      
    if (c == '3' || c == '4'){
      for (int a = 0; a < 1023; a++ ){
        EEPROM.write(a, 0);
      }
      Serial.println("EEPROM deleted");
    }
    if (c == '4'){
      toCoffeemaker("?M3\r\n");
      delay(10);
      if (fromCoffeemaker() == "?ok\r\n"){
        Serial.println("Inkasso mode activated");
      } else {
        Serial.print("error: coffeemaker not responding");
      }
    }     
    if (c == '5'){
      toCoffeemaker("?M1\r\n");
      delay(10);
      if (fromCoffeemaker() == "?ok\r\n"){
        Serial.println("Inkasso mode deactivated");
      } else {
        Serial.print("error: coffeemaker not responding");
      }
    }
    if (c == '6'){
      Serial.println();
      for (int i = 0; i <= 0x7F; i++ ){  // 100 nur testweise
        String outputString = "RE:";
//        if (a < 10){
//          outputString += "000";
//        } 
//        else if (a < 100) {
//          outputString += "00";
//        }
//        else if (a < 1000){
//          // outputString="";
//          outputString += "0";
//        }
        if(i <= 0xF){
          outputString += "0";
        }
        outputString += String(i, HEX);
        outputString.toUpperCase();
        outputString += "\r\n";
        toCoffeemaker(outputString);
//        Serial.print(outputString);
        delay(50);  // wait for answer?
        String inputString = fromCoffeemaker();
        inputString.remove(0,3);
        inputString.remove(4); 
//        String Hex = inputString.charAt(2)+""+inputString.charAt(3);
//        Hex += inputString.charAt(0)+""+inputString.charAt(1);
//        byte hex = byte(Hex);
        if (((i+8)%8) == 0){
          Serial.print("00");
          if(i<=0xF) Serial.print("0");
          Serial.print(i, HEX);
          Serial.print("  ");
        }
        Serial.print(inputString);
        Serial.print(" ");
        if (((i+1)%8) == 0){
          Serial.println();
        }
        delay(100);
      }
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
//  Serial.println();
//  Serial.print(outputString);
//  Serial.println(outputString.length());
  for (byte a = 0; a < outputString.length(); a++){
//    Serial.print(outputString.charAt(a));
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
