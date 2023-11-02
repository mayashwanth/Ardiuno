#include <SPI.h>
#include <MFRC522.h>
#include "SoftwareSerial.h"
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
SoftwareSerial ser(2,3); // RX, TX
void setup()
{
 Serial.begin(9600);   // Initiate a serial communication
  ser.begin (9600);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  //Serial.println();
}
void loop()
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
 String content= "";
 String coverted ="";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
   content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
   content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();
  for (char c : content)
  {
        if (c != ' ') {
            coverted += c;
        }
  }
  fun(coverted);
   // Serial.println(coverted);
  /*if (coverted == "0466C58ABB1490" ) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("1504,"+coverted);
    ser.write(1);
    //Serial.println();
    delay(3000);
  }*/
}
void fun(String str)
{
  Serial.println("1504,"+str);
  delay(3000);
}