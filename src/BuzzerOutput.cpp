#include "BuzzerOutput.h"
#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h> 
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800);

Adafruit_ILI9341 tft1 = Adafruit_ILI9341(TFT_CS_1, TFT_DC, TFT_RST);
Adafruit_ILI9341 tft2 = Adafruit_ILI9341(TFT_CS_2, TFT_DC, TFT_RST);

DFRobotDFPlayerMini myDFPlayer;  

// Separated timers and flags for each player
unsigned long rainbowTimer1 = 0;      
unsigned long rainbowTimer2 = 0;      
bool isRainbowActive1 = false;        
bool isRainbowActive2 = false;        
long rainbowHue1 = 0;                  
long rainbowHue2 = 0;                  
bool isMp3Online = false; 

// Internal Helper Functions

void setLEDColor(int playerIndex, uint32_t color) {
  // NEW: Logic to determine which strip to light up
  if (playerIndex == 1 || playerIndex == 0) {
    for(uint16_t i = 0; i < strip1.numPixels(); i++) strip1.setPixelColor(i, color);
    strip1.show();
  }
  if (playerIndex == 2 || playerIndex == 0) {
    for(uint16_t i = 0; i < strip2.numPixels(); i++) strip2.setPixelColor(i, color);
    strip2.show();
  }
}

void updateRainbowEffect(int playerIndex) {
  // NEW: Only updates the specific player's strip
  if (playerIndex == 1) {
    if (millis() - rainbowTimer1 > 20) { 
      rainbowTimer1 = millis();
      for(uint16_t i = 0; i < strip1.numPixels(); i++) {
        int pixelHue = rainbowHue1 + (i * 65536L / strip1.numPixels());
        strip1.setPixelColor(i, strip1.gamma32(strip1.ColorHSV(pixelHue)));
      }
      strip1.show();
      rainbowHue1 += 256; 
      if(rainbowHue1 >= 65536) rainbowHue1 = 0;
    }
  }
  if (playerIndex == 2) {
    if (millis() - rainbowTimer2 > 20) { 
      rainbowTimer2 = millis();
      for(uint16_t i = 0; i < strip2.numPixels(); i++) {
        int pixelHue = rainbowHue2 + (i * 65536L / strip2.numPixels());
        strip2.setPixelColor(i, strip2.gamma32(strip2.ColorHSV(pixelHue)));
      }
      strip2.show();
      rainbowHue2 += 256; 
      if(rainbowHue2 >= 65536) rainbowHue2 = 0;
    }
  }
}

void updateScreenUI(int playerIndex, String header, String subtext, uint16_t headerColor, uint16_t subtextColor) {
  // NEW: Logic to draw on Screen 1, Screen 2, or both
  if (playerIndex == 1 || playerIndex == 0) {
    tft1.fillScreen(ILI9341_BLACK); 
    tft1.setCursor(10, 40);
    tft1.setTextColor(headerColor);
    tft1.setTextSize(3); 
    tft1.print(header);
    tft1.setCursor(10, 110);
    tft1.setTextColor(subtextColor);
    tft1.setTextSize(5); 
    tft1.print(subtext);
  }
  if (playerIndex == 2 || playerIndex == 0) {
    tft2.fillScreen(ILI9341_BLACK); 
    tft2.setCursor(10, 40);
    tft2.setTextColor(headerColor);
    tft2.setTextSize(3); 
    tft2.print(header);
    tft2.setCursor(10, 110);
    tft2.setTextColor(subtextColor);
    tft2.setTextSize(5); 
    tft2.print(subtext);
  }
}

// System Initialization

void initOutputs() {
  strip1.begin(); strip1.show(); 
  strip2.begin(); strip2.show(); 
  setLEDColor(0, strip1.Color(0,0,0));
  
  tft1.begin(); tft1.setRotation(1); tft1.fillScreen(ILI9341_BLACK);
  tft2.begin(); tft2.setRotation(1); tft2.fillScreen(ILI9341_BLACK); 

  Serial1.begin(9600);
  Serial.println("SYS: Initializing DFPlayer Audio Module...");
  
  // Verify hardware handshake to prevent blocking functions later
  if (!myDFPlayer.begin(Serial1)) {
    Serial.println("ERR: DFPlayer Not Found. Enabling UART failsafe (Simulation Mode).");
    isMp3Online = false; 
  } else {
    Serial.println("SYS: DFPlayer Online.");
    myDFPlayer.volume(25);  
    isMp3Online = true;
  }
  
  triggerInitialSettings(0); 
}

void updateOutputs() {
  if (isRainbowActive1) updateRainbowEffect(1); // NEW: check P1
  if (isRainbowActive2) updateRainbowEffect(2); // NEW: check P2
}

//Even Triggers

void triggerInitialSettings(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(0, 0, 0)); 
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
  updateScreenUI(playerIndex, "SYSTEM STATUS", "READY", ILI9341_WHITE, ILI9341_LIGHTGREY);
}
 
void triggerWhiteSetting(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(255, 255, 255));
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
  
  // Implements a newline character to maintain textual bounds on the 320px x-axis
  updateScreenUI(playerIndex, "SYSTEM STATUS", "PLAYER\nDETECTED", ILI9341_WHITE, ILI9341_CYAN);
}

void triggerBlueSetting(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(0, 0, 255));
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
}

void triggerOrangeSetting(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(255, 165, 0));
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
}

void triggerStartQuestion(int playerIndex) {
  updateScreenUI(playerIndex, "READING QUESTION", "MIC MUTED", ILI9341_ORANGE, ILI9341_DARKGREY);
}

void triggerFloorClaimed(int playerIndex) { 
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = true;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = true;        
  updateScreenUI(playerIndex, "FLOOR CLAIMED", "5 sec", ILI9341_YELLOW, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(1); //Calling MP3 File
}

void triggerCorrectAnswer(int playerIndex, int newPoints) {
  setLEDColor(playerIndex, strip1.Color(0, 255, 0)); 
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
  
  String pointText = "+" + String(newPoints) + " PTS";
  updateScreenUI(playerIndex, "CORRECT!", pointText, ILI9341_GREEN, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(2);  //Calling MP3 File
}

// NEW: Silent Lockout for the losing player (no sound)
void triggerLockedOut(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(255, 0, 0)); // Turn LEDs Red
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
  
  // Update UI to show they are disabled
  updateScreenUI(playerIndex, "SYSTEM STATUS", "LOCKED OUT", ILI9341_RED, ILI9341_WHITE);
  
  // NOTE: myDFPlayer.play is NOT called here to prevent audio clashing
}

// Keep this for when someone actually gives a wrong answer or the timer hits 0
void triggerWrongOrTimeout(int playerIndex) {
  setLEDColor(playerIndex, strip1.Color(255, 0, 0)); 
  if (playerIndex == 1 || playerIndex == 0) isRainbowActive1 = false;
  if (playerIndex == 2 || playerIndex == 0) isRainbowActive2 = false;
  
  updateScreenUI(playerIndex, "LOCKED OUT", "WRONG", ILI9341_RED, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(3);  // Plays the "Wrong" sound
}

void updateLCDTimer(int playerIndex, int remainingSeconds) {
  // NEW: Checks which screen to update for the timer
  if (playerIndex == 1 || playerIndex == 0) {
      tft1.fillRect(10, 110, 200, 50, ILI9341_BLACK); 
      tft1.setCursor(10, 110);
      tft1.setTextColor(remainingSeconds <= 2 ? ILI9341_RED : ILI9341_WHITE);
      tft1.setTextSize(5);
      tft1.print(remainingSeconds); tft1.print(" sec");
  }
  if (playerIndex == 2 || playerIndex == 0) {
      tft2.fillRect(10, 110, 200, 50, ILI9341_BLACK); 
      tft2.setCursor(10, 110);
      tft2.setTextColor(remainingSeconds <= 2 ? ILI9341_RED : ILI9341_WHITE);
      tft2.setTextSize(5);
      tft2.print(remainingSeconds); tft2.print(" sec");
  }
}