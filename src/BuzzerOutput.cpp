#include "BuzzerOutput.h"
#include <Adafruit_NeoPixel.h>
#include <DFRobotDFPlayerMini.h> 
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
DFRobotDFPlayerMini myDFPlayer;  
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

unsigned long rainbowTimer = 0;      
bool isRainbowActive = false;        
long rainbowHue = 0;                  
bool isMp3Online = false; 

// Internal Helper Functions

void setLEDColor(uint32_t color) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void updateRainbowEffect() {
  if (millis() - rainbowTimer > 20) { 
    rainbowTimer = millis();
    for(uint16_t i = 0; i < strip.numPixels(); i++) {
      int pixelHue = rainbowHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    rainbowHue += 256; 
    if(rainbowHue >= 65536) rainbowHue = 0;
  }
}

void updateScreenUI(String header, String subtext, uint16_t headerColor, uint16_t subtextColor) {
  tft.fillScreen(ILI9341_BLACK); 
  
  tft.setCursor(10, 40);
  tft.setTextColor(headerColor);
  tft.setTextSize(3); 
  tft.print(header);
  
  tft.setCursor(10, 110);
  tft.setTextColor(subtextColor);
  tft.setTextSize(5); 
  tft.print(subtext);
}

// System Initialization

void initOutputs() {
  // 1. Initialize Solid-State Lighting
  strip.begin();
  strip.setBrightness(100); 
  strip.show(); 
  
  // 3. Initialize TFT Display Bus
  tft.begin();
  tft.setRotation(1); // Landscape orientation (320x240)
  tft.fillScreen(ILI9341_BLACK);

  // 4. Initialize UART Audio Module
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
  

  triggerInitialSettings(); 
}

void updateOutputs() {
  if (isRainbowActive) {
    updateRainbowEffect();
  }
}

//Even Triggers

void triggerInitialSettings() {
  setLEDColor(strip.Color(0, 0, 0)); 
  isRainbowActive = false;
  updateScreenUI("SYSTEM STATUS", "READY", ILI9341_WHITE, ILI9341_LIGHTGREY);
}
 
void triggerWhiteSetting() {
  setLEDColor(strip.Color(255, 255, 255));
  isRainbowActive = false;
  
  // Implements a newline character to maintain textual bounds on the 320px x-axis
  updateScreenUI("SYSTEM STATUS", "PLAYER\nDETECTED", ILI9341_WHITE, ILI9341_CYAN);
}

void triggerBlueSetting() {
  setLEDColor(strip.Color(0, 0, 255));
  isRainbowActive = false;
}

void triggerOrangeSetting() {
  setLEDColor(strip.Color(255, 165, 0));
  isRainbowActive = false;
}

void triggerStartQuestion() {
  updateScreenUI("READING QUESTION", "MIC MUTED", ILI9341_ORANGE, ILI9341_DARKGREY);
}

void triggerFloorClaimed() { 
  isRainbowActive = true;        //Continuous execution of updateRainbowEffect()
  updateScreenUI("FLOOR CLAIMED", "5 sec", ILI9341_YELLOW, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(1); //Calling MP3 File
}

void triggerCorrectAnswer(int newPoints) {
  setLEDColor(strip.Color(0, 255, 0)); 
  isRainbowActive = false;
  
  String pointText = "+" + String(newPoints) + " PTS";
  updateScreenUI("CORRECT!", pointText, ILI9341_GREEN, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(2);  //Calling MP3 File
}

void triggerWrongOrTimeout() {
  setLEDColor(strip.Color(255, 0, 0)); 
  isRainbowActive = false;
  updateScreenUI("LOCKED OUT", "WRONG", ILI9341_RED, ILI9341_WHITE);
  
  if (isMp3Online) myDFPlayer.play(3);  //Calling MP3 File
}

void updateLCDTimer(int remainingSeconds) {
  tft.fillRect(10, 110, 200, 50, ILI9341_BLACK); 
  tft.setCursor(10, 110);
  
  if (remainingSeconds <= 2) {
    tft.setTextColor(ILI9341_RED);
  } else {
    tft.setTextColor(ILI9341_WHITE);
  }
  
  tft.setTextSize(5);
  tft.print(remainingSeconds);
  tft.print(" sec"); 
}