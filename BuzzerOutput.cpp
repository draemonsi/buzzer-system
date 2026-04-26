#include "BuzzerOutput.h"

// Note: Added two libraries for led strip and display

#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>

// --- OBJECT INITIALIZATION ---
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- INTERNAL VARIABLES ---
unsigned long rainbowTimer = 0;
bool isRainbowActive = false;
long rainbowHue = 0;

//        INTERNAL HELPER FUNCTIONS

// Quickly sets the entire LED strip to a single color
void setLEDColor(uint32_t color) {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Handles the non-blocking rainbow animation
void updateRainbowEffect() {
  // Uses millis() instead of delay() so it doesn't freeze the rest of the code
  // Updates the frame every 20ms
  if (millis() - rainbowTimer > 20) { 
    rainbowTimer = millis();
    for(int i = 0; i < strip.numPixels(); i++) {
      // 65536 is the max hue in Adafruit_NeoPixel. This math spreads the colors evenly.
      int pixelHue = rainbowHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    rainbowHue += 256; // Shift the colors for the next frame
    if(rainbowHue >= 65536) rainbowHue = 0;
  }
}

//          PUBLIC FUNCTIONS

void initOutputs() {
  // 1. Initialize LEDs
  strip.begin();
  strip.setBrightness(100); 
  strip.show(); 

  // 2. Initialize Mic Relay & Buzzer
  pinMode(MIC_RELAY, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // 3. Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.println("System Online (MP3 Disabled for Simulation).");
  
  triggerInitialSettings(); 
}

void updateOutputs() {
  // Only animate the LEDs if a contestant currently has the floor
  if (isRainbowActive) {
    updateRainbowEffect();
  }
}

// --- The Flowchart Triggers ---

void triggerInitialSettings() {
  digitalWrite(MIC_RELAY, LOW); // LOW turns the Relay OFF (Mic muted)
  setLEDColor(strip.Color(0, 0, 0)); // Turn off LEDs
  isRainbowActive = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready    ");
}
 
void triggerWhiteSetting() {
  digitalWrite(MIC_RELAY, LOW); // Contestant Mic OFF
  setLEDColor(strip.Color(255, 255, 255));
  isRainbowActive = false;
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
  digitalWrite(MIC_RELAY, LOW); // Host is talking, ensure contestant is muted
}

void triggerFloorClaimed() {
  digitalWrite(MIC_RELAY, HIGH); // HIGH turns Relay ON (Contestant can speak!)
  isRainbowActive = true;        // Start the rainbow animation
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Answer Timer:");

  // Play a quick 1000Hz notification beep
  tone(BUZZER_PIN, 1000); 
  delay(200);
  noTone(BUZZER_PIN);
}

void triggerCorrectAnswer(int newPoints) {
  digitalWrite(MIC_RELAY, LOW); // Round over, mute contestant
  setLEDColor(strip.Color(0, 255, 0)); // Green for correct
  isRainbowActive = false;
  
  // Play a happy double-beep
  tone(BUZZER_PIN, 1500);
  delay(100);
  noTone(BUZZER_PIN);
  delay(50);
  tone(BUZZER_PIN, 1500);
  delay(100);
  noTone(BUZZER_PIN);
  
  // Update the scoreboard
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Points: ");
  lcd.print(newPoints);
}

void triggerWrongOrTimeout() {
  digitalWrite(MIC_RELAY, LOW); // Round over, mute contestant
  setLEDColor(strip.Color(255, 0, 0)); // Red for wrong
  isRainbowActive = false;

  // Play a low, long error buzz
  tone(BUZZER_PIN, 400); 
  delay(500);
  noTone(BUZZER_PIN);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Locked Out      ");
}

void updateLCDTimer(int remainingSeconds) {
  lcd.setCursor(0, 1);
  lcd.print(remainingSeconds);
  lcd.print("s remaining   "); 
}