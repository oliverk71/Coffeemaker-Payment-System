#include <SoftwareSerial.h>
#include <EEPROM.h>

SoftwareSerial myCoffeemaker(10,11); // RX, TX

void setup() {
  Serial.begin(9600);
  myCoffeemaker.begin(9600);
  Serial.println("(r)ead or (d)elete all data in EEPROM or (i)nitialize) for CoffeemakerPM (deletes EEPROM and activates Inkasso mode). To deactivate Inkasso mode press 'v'.");
}

void loop() {
  if (Serial.available()){
    char c = Serial.read();
    if (c == 'r'){
      for (int a = 0; a < 1024; a+=16){
        for (int a2 = 0; a < 16; a++){
          byte b = EEPROM.read(a+a2);
          Serial.print(b, HEX);
          Serial.print(" ");
        }
        Serial.println();
      }  
    }
      
    if (c == 'd' || c == 'i'){
      for (int a = 0; a < 1023; a++ ){
        EEPROM.write(a, 0);
      }
      Serial.println("EEPROM deleted");
    }
    if (c == 'i'){
      toCoffeemaker("?M3\r\n");
      delay(10);
      if (fromCoffeemaker() == "?ok\r\n"){
        Serial.println("Inkasso mode activated");
      } else {
        Serial.print("error: coffeemaker not responding");
      }
    }     
    if (c == 'v'){
      toCoffeemaker("?M1\r\n");
      delay(10);
      if (fromCoffeemaker() == "?ok\r\n"){
        Serial.println("Inkasso mode deactivated");
      } else {
        Serial.print("error: coffeemaker not responding");
      }
    }       
  }    
}



String fromCoffeemaker(){
  delay(20); // testweise
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
  delay(20); // testweise
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
