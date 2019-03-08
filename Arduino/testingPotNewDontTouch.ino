#include <SPI.h>  
#include <SoftwareSerial.h>
SoftwareSerial ble(9,8); // RX, TX
float batteryOffset = 0;
float currentOffset = 0;

int currentPotLevel = 0;

const int CS = 10;
int PotWiperVoltage = 1;
int RawVoltage = 0;
float Voltage = 0;
 
void setup() {
  pinMode (CS, OUTPUT);  
  Serial.begin(9600);
  ble.begin(9600);
  SPI.begin();  
  MCP41010Write(currentPotLevel); 
  analogReference(INTERNAL); 
}
 
void loop() {
  int byteCounter = 0;
  int bytes[] = {0,0};
  boolean isOffsetMessage = false;

  if (ble.available()) {
    while (ble.available()) {
      bytes[byteCounter] = ble.read();
      if (bytes[byteCounter] == 100) {
        isOffsetMessage = true;
        byteCounter = -1;
      }
      byteCounter += 1;
    }

    float newCurrent = 0.0;
    float decimal = 0.0;
    decimal += bytes[1];
    newCurrent += bytes[0];
    Serial.print(decimal/100);
    newCurrent += (decimal/100.0);
    Serial.print("Updating Current To "); Serial.println(newCurrent);

    if (isOffsetMessage) {
      Serial.print("Got Offset of: "); Serial.println(newCurrent);
      currentOffset = newCurrent;
    } else {
        Serial.print("Got New Current of: "); Serial.println(newCurrent);
        if (newCurrent < 20) {
          setCurrent(newCurrent);
        }
    }


    byteCounter = 0;
  }

  // read the input on analog pin 0:
  int batteryValue = analogRead(2);
  int currentValue = analogRead(0);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float batteryVoltage = (batteryValue * ((1.1 / 1023.0)*100.0));
  int integerVoltage = batteryVoltage;
  float current =  (((currentValue * (1.1 / 1023.0))*23.0)-1.77);
  int integerCurrent = current;

   // print out the value you read:
  Serial.print("Voltage: ");Serial.println(batteryVoltage, 5);
  Serial.print("Current: ");Serial.println(current, 5);

  float battDecimal = (batteryVoltage - (integerVoltage))*100;
  float currentDecimal = (current - (integerCurrent))*100;
  //Serial.print(battDecimal);

  ble.write(batteryVoltage);
  ble.write(battDecimal);
  ble.write(current);
  ble.write(currentDecimal);

  Serial.println("Sent Bluetooth");

  delay(100);

}

void setCurrent(float newCurrent) {
  boolean didGetToCurrent = false;

  if (newCurrent < 0.2) {
    MCP41010Write(0);
    currentPotLevel = 0;
  } else {
    int currentValue = analogRead(0);
    float current =  (((currentValue * (1.1 / 1023.0))*23.0)-1.77 + currentOffset);

    if (newCurrent > current) {

      while (didGetToCurrent != true) {
        MCP41010Write(currentPotLevel);
        delay(50);

        int currentValue = analogRead(0);
        float current =  (((currentValue * (1.1 / 1023.0))*23.0)-1.77 + currentOffset);

        if ((newCurrent - current) < 0.13) {
          didGetToCurrent = true;
          MCP41010Write(currentPotLevel);
          Serial.print("Level Set: "); Serial.println(currentPotLevel);
        } else {
          currentPotLevel += 1;
        }
      }
      
      
    } else if (newCurrent < current) {

      while (didGetToCurrent != true) {
        MCP41010Write(currentPotLevel);
        delay(50);

        int currentValue = analogRead(0);
        float current =  (((currentValue * (1.1 / 1023.0))*23.0)-1.77 + currentOffset);

        if ((current - newCurrent) < 0.13) {
          didGetToCurrent = true;
          MCP41010Write(currentPotLevel);
          Serial.print("Level Set: "); Serial.println(currentPotLevel);
        } else {
          currentPotLevel -= 1;
        }
      
    }

  }
  }

  
}

void MCP41010Write(byte value) 
{
  // Note that the integer vale passed to this subroutine
  // is cast to a byte
  
  digitalWrite(CS,LOW);
  SPI.transfer(B00010001); // This tells the chip to set the pot
  SPI.transfer(value);     // This tells it the pot position
  digitalWrite(CS,HIGH); 

  
}
