
/*********************************************************************
This is an example for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

// This version uses the internal data queing so you can treat it like Serial (kinda)!
#include <SoftwareSerial.h>
#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include "Adafruit_Soundboard.h"

#define SFX_TX 5
#define SFX_RX 6
#define SFX_RST 8

// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK = 13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 3     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9

// we'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third 
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

unsigned int lastDebounceTime = 0;
bool trackNumber = false;
char c;
uint16_t vol = 204;
bool temp = false;

/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{
  ss.begin(9600);
  BTLEserial.begin();

  if (!sfx.reset()) {
    while (1);
  }

  delay(2000);
  PlayWelcomeSentence();
  
  
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void VolDown(){
      Serial.println("Vol down...");
      uint16_t v;
      for(int i=0; i<3; i++){
        if (! (v=sfx.volDown()) ) {
          Serial.println("Failed to adjust");
          //sfx.reset();
        }
        
      }
      Serial.print("Volume: "); 
      Serial.println(v);
      lastDebounceTime = millis();
}

void VolUp(){
      Serial.println("Vol up...");
      uint16_t v;
      for(int i=0; i<3; i++){
        if (! (v=sfx.volUp()) ) {
          Serial.println("Failed to adjust");
          //sfx.reset();
        }
        
      }
      Serial.print("Volume: "); 
      Serial.println(v);
      lastDebounceTime = millis();
}

void PlayBlib (){
      if (! sfx.playTrack((uint8_t)2) ) {
        Serial.println("Failed to play track?");
        //sfx.reset();
      }
}
void PlayTrackOnce(){
  sfx.playTrack((uint8_t)trackNumber);
}

void PlayWelcomeSentence(){
  sfx.playTrack((uint8_t)3);
}

void PlayTrack(){
    while(true){
      delay(2000);
      BTLEserial.pollACI();
      aci_evt_opcode_t status = BTLEserial.getState();
      if (status == ACI_EVT_CONNECTED) {
        c = BTLEserial.read();
        if(c == 'D')
          VolDown();
  
        if(c == 'U')
          VolUp();
  
        if(c == 'M'){
          sfx.stop();
          break;
        }
        else{
          sfx.playTrack((uint8_t)trackNumber) ;
          if(!trackNumber)
            delay(5000);
           else
            delay(1000);
        }
        
      }
    }
}


void loop()
{

  // Tell the nRF8001 to do whatever it should be working on.
  BTLEserial.pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
      //play connected track
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    // OK while we still have something to read, get a character and print it out
    while (BTLEserial.available()) {
        c = BTLEserial.read();
        Serial.print(c);Serial.print('\n');
      
      
      if(c == 'D'){
        VolDown();
        delay(100);
        PlayBlib ();
      }

      if(c == 'U'){
        VolUp();
        delay(100);
        PlayBlib ();
      }

      if(c == 'P')
        PlayTrack();
        
      if(c == 'C'){
        trackNumber = !trackNumber;
        delay(1000);
        PlayTrackOnce();
      }

      if(c == 'R'){
        sfx.reset();
      }
    
    }
//    // Next up, see if we have any data to get from the Serial console
//
//    if (Serial.available()) {
//      // Read a line from Serial
//      Serial.setTimeout(100); // 100 millisecond timeout
//      String s = Serial.readString();
//
//      // We need to convert the line to bytes, no more than 20 at this time
//      uint8_t sendbuffer[20];
//      s.getBytes(sendbuffer, 20);
//      char sendbuffersize = min(20, s.length());
//
//      Serial.print(F("\n* Sending -> \"")); Serial.print((char *)sendbuffer); Serial.println("\"");
//
//      // write the data
//      BTLEserial.write(sendbuffer, sendbuffersize);
//    }
  }
}




