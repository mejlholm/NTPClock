/**
 * Made for DOIT ESP32 DEVKV1 - Shows clock on a 4x8x8 LED Max7219 unit.
 */

#include "sprites.h"
#include "setup.h"
#include "LedControl.h"
#include <WiFi.h>
#include <ezTime.h>

// Setup timezone
Timezone myTZ;

LedControl lc = LedControl(DIN_PORT, CLK_PORT, CS_PORT, displays);

const int onboardLedPin=2;

/* array to convert int into sprite */
byte* convTable[10] = {sprite_0, sprite_1, sprite_2, sprite_3, sprite_4, sprite_5, sprite_6, sprite_7, sprite_8, sprite_9};

/* array to loop over ssid/passwords */
char* ssids[3] = {ssid1, ssid2, ssid3};
char* passwords[3] = {password1, password2, password3};

/* canvas aka buffer of what to draw */
int nextRow = 0;
byte canvas[32] = {n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n};

// wake up displays and clear them
void prepareDisplay(int i) {
  lc.shutdown(i,false);
  lc.setIntensity(i,0);
  lc.clearDisplay(i);    
}

// show while connecting to wifi
void connectWifi() {
  drawWifi();

  for (int i = 0; i < 3; i++) {
    bool connected = false;
    connected = tryWifiConnection(ssids[i], passwords[i]);
    if (connected == true) {
      return;
    }    
  }  
}

bool tryWifiConnection(char* ssid, char* password) {
  int attempts = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(400);
    attempts += 1;
  }
  return WiFi.status() == WL_CONNECTED;
}

//show while connecting to NTP server
void connectTime() {
  drawTime();
  
  myTZ.setLocation(LOCATION);
  waitForSync();
}

// flash onboard led
void blinkOnboardLed(int value) {
    digitalWrite(onboardLedPin, value);
}

// get the tens of timeunit
int getTens(int timeUnit) {
  return timeUnit / 10;
}

// get the ones of timeunit
int getOnes(int timeUnit) {
  return timeUnit % 10;
}

void addToCanvas(int size, byte* sprite) {
  for (int idx = 0; idx < size; idx++) {
    canvas[nextRow] = sprite[idx];
    nextRow++;
  }

  canvas[nextRow] = n;
  nextRow++;
}

void drawDisplay(int display, int offset) {
  for (int idx = 0; idx < 8; idx++) {
    lc.setColumn(display, idx, canvas[offset + idx]);
  }  
}

void drawCanvas() {
  drawDisplay(3, 0);
  drawDisplay(2, 8);
  drawDisplay(1, 16);
  drawDisplay(0, 24);

  //reset display
  nextRow = 0;
}

void indent(int level) {
  for (int i = 0; i < level; i++) {
    canvas[nextRow + i] = n;
  }
  nextRow = level;
}

void drawCurrentTime(){
  tmElements_t tm;
  breakTime(myTZ.now(), tm);

  //indent to center on the display
  indent(1);

  addToCanvas(6, convTable[getTens(tm.Hour)]);
  addToCanvas(6, convTable[getOnes(tm.Hour)]);

  if (!COLON_BLINK || tm.Second % 2 == 1) {
    addToCanvas(2, sprite_colon);  
  } else {
    addToCanvas(2, sprite_blank);  
  }
  
  addToCanvas(6, convTable[getTens(tm.Minute)]);
  addToCanvas(6, convTable[getOnes(tm.Minute)]);

  drawCanvas();
}

void drawWifi(){
  indent(4);

  addToCanvas(7, sprite_w);  
  addToCanvas(4, sprite_i);  
  addToCanvas(6, sprite_f);  
  addToCanvas(4, sprite_i);  

  drawCanvas();
}

void drawTime(){
  indent(3);

  addToCanvas(6, sprite_t);  
  addToCanvas(4, sprite_i);  
  addToCanvas(7, sprite_m);  
  addToCanvas(6, sprite_e);  

  drawCanvas();
}

// general setup
void setup() {   
  pinMode(onboardLedPin, OUTPUT);

  delay(2000); //avoid flickering when flashing the esp32
  for (int i = 0; i < displays; i++) {
    prepareDisplay(i);
  }

  connectWifi();
  connectTime();
}

// main loop
void loop() {
  drawCurrentTime();

  delay(200);
}
