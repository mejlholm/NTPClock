/**
 * Made for DOIT ESP32 DEVKV1 - Shows clock on a 4x8x8 LED Max7219 unit.
 */

#include "LedControl.h"
#include <WiFi.h>
#include <ezTime.h>

// Replace with your network credentials
char* ssid     = "";
char* password = "";

// Setup led control library
int displays=4;
LedControl lc = LedControl(SCK,MISO,MOSI,displays);

const int pixels=8;
const int onboardLedPin=2;

// Setup timezone
Timezone myTZ;


/* sprites */
byte n                     = B00000000; //nothing
byte n0[pixels/2]          = {n, B01111110, B01000010, B01111110}; 
byte n1[pixels/2]          = {n, n, B01111110, n};
byte n2[pixels/2]          = {n, B01011110, B01010010, B01110010};
byte n3[pixels/2]          = {n, B01000010, B01010010, B01111110}; 
byte n4[pixels/2]          = {n, B01110000, B00010000, B01111110};
byte n5[pixels/2]          = {n, B01110010, B01010010, B01011110}; 
byte n6[pixels/2]          = {n, B01111110, B01010010, B01011110};
byte n7[pixels/2]          = {n, B01000000, B01000000, B01111110};
byte n8[pixels/2]          = {n, B01111110, B01010010, B01111110};
byte n9[pixels/2]          = {n, B01110010, B01010010, B01111110};
byte colon[pixels/2]       = {n, n, B00100100, n};
byte exclamation[pixels/2] = {n, n, B01111010, n};
byte blank[pixels/2]       = {B00000000, B00000000, B00000000, B00000000};
byte le[pixels/2]          = {B01111110, B01010010, B01010010, B01000010};
byte lf[pixels/2]          = {B01111110, B01010000, B01010000, B01000000};
byte li[pixels/2]          = {n, B01000010, B01111110, B01000010};
byte li2[pixels/2]         = {B01000010, B01111110, B01000010, n};
byte lm_first[pixels/2]    = {B00000000, B00000000, B00111110, B01000000};
byte lm[pixels/2]          = {B00110000, B01000000, B00111110, n};
byte lt_first[pixels/2]    = {n, n, n, B01000000};
byte lt[pixels/2]          = {B01000000, B01111110, B01000000, B01000000};
byte lw_first[pixels/2]    = {B00000000, B00000000, B01111100, B00000010};
byte lw[pixels/2]          = {B00001100, B00000010, B01111100, n};

/* table to convert int into sprite */
byte* convTable[10] = {n0, n1, n2, n3, n4, n5, n6, n7, n8, n9};

/* target sprites for what to show */
byte* sprite0 = blank;
byte* sprite1 = blank;
byte* sprite2 = blank;
byte* sprite3 = blank;
byte* sprite4 = blank;
byte* sprite5 = blank;
byte* sprite6 = blank;
byte* sprite7 = blank;
byte* oldSprite0 = blank;
byte* oldSprite1 = blank;
byte* oldSprite2 = blank;
byte* oldSprite3 = blank;
byte* oldSprite4 = blank;
byte* oldSprite5 = blank;
byte* oldSprite6 = blank;
byte* oldSprite7 = blank;

// wake up displays and clear them
void prepareDisplay(int i) {
  lc.shutdown(i,false);
  lc.setIntensity(i,0);
  lc.clearDisplay(i);    
}

// show while connecting to wifi
void connectWifi() {
  drawWifi();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    drawExclamation();
    delay(400);
  }
}

//show while connecting to NTP server
void connectTime() {
  drawTime();

  myTZ.setLocation("Europe/Copenhagen");
  setInterval(60);

  waitForSync();

}

// general setup
void setup() {   
  pinMode(onboardLedPin, OUTPUT);

  delay(2000);
  for (int i = 0; i < displays; i++) {
    prepareDisplay(i);
  }

  connectWifi();
  connectTime();
}

// show time
void drawTime() {
  sprite0 = blank;
  sprite1 = le;
  sprite2 = lm;
  sprite3 = lm_first;
  sprite4 = li;
  sprite5 = lt;
  sprite6 = lt_first;
  sprite7 = blank;

  displayClock();  
}

// show wifi
void drawWifi() {
  sprite0 = blank;
  sprite1 = li;
  sprite2 = lf;
  sprite3 = li2;
  sprite4 = lw;
  sprite5 = lw_first;
  sprite6 = blank;
  sprite7 = blank;

  displayClock();
}

// toggle exclamation mark
void drawExclamation() {

    if (sprite0 == exclamation) {
      sprite0 = blank;
      blinkOnboardLed(LOW);
    } else {
      sprite0 = exclamation;
      blinkOnboardLed(HIGH);
    }
    
    displayClock();  
}

// flash onboard led
void blinkOnboardLed(int value) {
    digitalWrite(onboardLedPin, value);
}

// main loop
void loop() {

  tmElements_t tm;
  breakTime(myTZ.now(), tm);

  sprite0 = convTable[getOnes(tm.Second)];
  sprite1 = convTable[getTens(tm.Second)];
  sprite2 = colon;
  sprite3 = convTable[getOnes(tm.Minute)];
  sprite4 = convTable[getTens(tm.Minute)];
  sprite5 = colon;
  sprite6 = convTable[getOnes(tm.Hour)];
  sprite7 = convTable[getTens(tm.Hour)];

  displayClock();

  delay(200);
}

// get the tens of timeunit
int getTens(int timeUnit) {
  return timeUnit / 10;
}

// get the ones of timeunit
int getOnes(int timeUnit) {
  return timeUnit % 10;
}

// display the sprites that have been setup
void displayClock() {

  byte* sprites[8] = {sprite0, sprite1, sprite2, sprite3, sprite4, sprite5, sprite6, sprite7};
  byte* oldSprites[8] = {oldSprite0, oldSprite1, oldSprite2, oldSprite3, oldSprite4, oldSprite5, oldSprite6, oldSprite7};

  byte* curSprite0;
  byte* curSprite1;
  byte* curOldSprite0;
  byte* curOldSprite1;

  for (int scroll = 0; scroll < pixels; scroll++) {  
    for (int address = 0; address < 4; address++) {

      int curAddr = address % 4;
      
      curSprite0 = sprites[curAddr*2];
      curSprite1 = sprites[(curAddr*2)+1];
      curOldSprite0 = oldSprites[curAddr*2];
      curOldSprite1 = oldSprites[(curAddr*2)+1];

      if (curOldSprite0 != curSprite0) {
        for (int idx = 0; idx < (pixels/2); idx++) {
          lc.setColumn(address, idx+4, (curOldSprite0[idx] << scroll+1 |curSprite0[idx] >> (pixels - scroll-1))); 
        }
      }
    
      if (curOldSprite1 != curSprite1){
        for (int idx = 0; idx < (pixels/2); idx++) {
          lc.setColumn(address, idx, (curOldSprite1[idx] << scroll+1 |curSprite1[idx] >> (pixels - scroll-1))); 
        }
      }
    }
    delay(25);
  }

  oldSprite0 = sprite0;
  oldSprite1 = sprite1;
  oldSprite2 = sprite2;
  oldSprite3 = sprite3;
  oldSprite4 = sprite4;
  oldSprite5 = sprite5;
  oldSprite6 = sprite6;
  oldSprite7 = sprite7;
}
