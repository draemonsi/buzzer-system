#include "esp32/ESP32_HostPanel.h"

// --- GLOBALS ---
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_FT6206 ts = Adafruit_FT6206(); 

// Buttons
Adafruit_GFX_Button btnPtsDec, btnPtsInc, btnTimeDec, btnTimeInc;
Adafruit_GFX_Button btnStart, btnCorrect, btnWrong;
Adafruit_GFX_Button btnTimeout, btnResetRnd, btnRestartGame;

// Game Variables
int pointsToAward = 1;
int timerLimit = 10;
int currentTimer = 10;
unsigned long lastTimerTick = 0;
bool timerRunning = false;

int p1Score = 0;
int p2Score = 0;
int activeBuzzer = 0; // 0=None, 1=P1, 2=P2

void setupHostPanel() {
  // Debug Serial via USB
  Serial.begin(9600);
  Serial.println("ESP32 System Booting...");

  // Communication Serial to Arduino Mega
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  tft.begin();
  tft.setRotation(1); // Landscape (320x240)
  tft.fillScreen(ILI9341_BLACK);
  
  // Initialize I2C Capacitive Touch
  if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen. Check I2C wiring (SDA=21, SCL=22).");
  } else {
    Serial.println("Touchscreen initialized successfully.");
  }

  // --- BUTTON INITIALIZATION (Strict 320x240 layout) ---
  // Row 1: Settings Adjustments (Y=125, Width=74)
  btnPtsDec.initButton(&tft, 40, 125, 74, 34, ILI9341_WHITE, ILI9341_DARKGREY, ILI9341_WHITE, "-1 PT", 2);
  btnPtsInc.initButton(&tft, 120, 125, 74, 34, ILI9341_WHITE, ILI9341_DARKGREY, ILI9341_WHITE, "+1 PT", 2);
  btnTimeDec.initButton(&tft, 200, 125, 74, 34, ILI9341_WHITE, ILI9341_DARKGREY, ILI9341_WHITE, "-5 SEC", 2);
  btnTimeInc.initButton(&tft, 280, 125, 74, 34, ILI9341_WHITE, ILI9341_DARKGREY, ILI9341_WHITE, "+5 SEC", 2);

  // Row 2: Main Flow Actions (Y=175, Width=98)
  btnStart.initButton(&tft, 53, 175, 98, 44, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "START", 2);
  btnCorrect.initButton(&tft, 160, 175, 98, 44, ILI9341_WHITE, ILI9341_GREEN, ILI9341_BLACK, "CORRECT", 2);
  btnWrong.initButton(&tft, 267, 175, 98, 44, ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE, "WRONG", 2);

  // Row 3: System Controls (Y=222, Width=98)
  btnTimeout.initButton(&tft, 53, 222, 98, 34, ILI9341_WHITE, ILI9341_ORANGE, ILI9341_BLACK, "TIMEOUT", 2);
  btnResetRnd.initButton(&tft, 160, 222, 98, 34, ILI9341_WHITE, ILI9341_MAROON, ILI9341_WHITE, "RST RND", 2);
  btnRestartGame.initButton(&tft, 267, 222, 98, 34, ILI9341_WHITE, ILI9341_PURPLE, ILI9341_WHITE, "RESTART", 2);

  drawMainWindow();
  
  Serial.println("Host Panel GUI Ready!");
}

void loopHostPanel() {
  processTouch();
  handleIncomingData();

  // Timer Tick Logic
  if(timerRunning && (millis() - lastTimerTick >= 1000)) {
    lastTimerTick = millis();
    if(currentTimer > 0) {
      currentTimer--;
      updateTimerDisplay();
      if(currentTimer == 0) {
        timerRunning = false;
        updateStatusDisplay("TIME UP!", ILI9341_RED);
        Serial2.println("CMD:TIMEOUT"); // Send to Mega
      }
    }
  }
}

void processTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  
  // Landscape Mapping for FT6206 Capacitive Touch
  int x = p.y; 
  int y = 240 - p.x;

  // --- Row 1: Adjust Settings ---
  if(btnPtsDec.contains(x, y)) { 
    if(pointsToAward > 1) pointsToAward--; 
    updateSettingsDisplay(); 
    delay(200); 
  }
  if(btnPtsInc.contains(x, y)) { 
    pointsToAward++; 
    updateSettingsDisplay(); 
    delay(200); 
  }
  if(btnTimeDec.contains(x, y)) { 
    if(timerLimit > 5) timerLimit -= 5; 
    currentTimer = timerLimit; 
    updateSettingsDisplay(); 
    updateTimerDisplay(); 
    delay(200); 
  }
  if(btnTimeInc.contains(x, y)) { 
    timerLimit += 5; 
    currentTimer = timerLimit; 
    updateSettingsDisplay(); 
    updateTimerDisplay(); 
    delay(200); 
  }

  // --- Row 2: Flow Actions ---
  if(btnStart.contains(x, y)) {
    Serial2.println("CMD:START"); // Send to Mega
    timerRunning = true;
    lastTimerTick = millis();
    updateStatusDisplay("QUESTION STARTED", ILI9341_GREEN);
    delay(200);
  }
  
  if(btnCorrect.contains(x, y)) {
    if(activeBuzzer == 1) p1Score += pointsToAward;
    else if(activeBuzzer == 2) p2Score += pointsToAward;
    
    Serial2.println("CMD:CORRECT"); // Send to Mega
    updateScoreDisplay();
    updateStatusDisplay("ANSWER CORRECT!", ILI9341_GREEN);
    timerRunning = false;
    activeBuzzer = 0;
    delay(200);
  }

  if(btnWrong.contains(x, y)) {
    Serial2.println("CMD:WRONG"); // Send to Mega
    updateStatusDisplay("ANSWER WRONG!", ILI9341_RED);
    timerRunning = false;
    activeBuzzer = 0;
    delay(200);
  }

  // --- Row 3: Resets & Overrides ---
  if(btnTimeout.contains(x, y)) {
    Serial2.println("CMD:TIMEOUT"); // Send to Mega
    updateStatusDisplay("FORCED TIMEOUT", ILI9341_ORANGE);
    timerRunning = false;
    delay(200);
  }

  if(btnResetRnd.contains(x, y)) {
    Serial2.println("CMD:RESET_ROUND"); // Send to Mega
    updateStatusDisplay("ROUND RESET", ILI9341_WHITE);
    timerRunning = false;
    currentTimer = timerLimit;
    activeBuzzer = 0;
    updateTimerDisplay();
    delay(200);
  }

  if(btnRestartGame.contains(x, y)) {
    Serial2.println("CMD:RESTART_GAME"); // Send to Mega
    updateStatusDisplay("GAME RESTARTED", ILI9341_PURPLE);
    timerRunning = false;
    currentTimer = timerLimit;
    activeBuzzer = 0;
    p1Score = 0;  // Reset P1 Score
    p2Score = 0;  // Reset P2 Score
    updateScoreDisplay();
    updateTimerDisplay();
    delay(300);
  }
}

void handleIncomingData() {
  // Listen to Serial2 for triggers from the Arduino Mega
  if(Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();
    
    // Echo to debug monitor so you can see what the Mega sent
    Serial.println("Received from Mega: " + msg); 
    
    if(msg == "BUZ:1") {
      activeBuzzer = 1;
      timerRunning = false; // Stop timer to give them time to answer
      updateStatusDisplay("PLAYER 1 BUZZED!", ILI9341_CYAN);
    } 
    else if(msg == "BUZ:2") {
      activeBuzzer = 2;
      timerRunning = false;
      updateStatusDisplay("PLAYER 2 BUZZED!", ILI9341_MAGENTA);
    }
  }
}

void drawMainWindow() {
  tft.fillScreen(ILI9341_BLACK);
  
  updateStatusDisplay("SYSTEM READY", ILI9341_WHITE);
  updateScoreDisplay();
  updateSettingsDisplay();
  updateTimerDisplay();
  
  // Draw all buttons
  btnPtsDec.drawButton();
  btnPtsInc.drawButton();
  btnTimeDec.drawButton();
  btnTimeInc.drawButton();
  
  btnStart.drawButton();
  btnCorrect.drawButton();
  btnWrong.drawButton();
  
  btnTimeout.drawButton();
  btnResetRnd.drawButton();
  btnRestartGame.drawButton();
}

void updateStatusDisplay(String status, uint16_t color) {
  tft.fillRect(0, 0, 320, 25, ILI9341_BLACK);
  tft.setCursor(10, 5);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.print(status);
}

void updateScoreDisplay() {
  tft.fillRect(0, 25, 320, 25, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  
  tft.setCursor(10, 30);
  tft.print("P1: "); tft.print(p1Score);
  
  tft.setCursor(170, 30);
  tft.print("P2: "); tft.print(p2Score);
}

void updateSettingsDisplay() {
  tft.fillRect(0, 50, 320, 25, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_YELLOW);
  
  tft.setCursor(10, 55);
  tft.print("AWARD: "); tft.print(pointsToAward);
  
  tft.setCursor(170, 55);
  tft.print("LIMIT: "); tft.print(timerLimit); tft.print("s");
}

void updateTimerDisplay() {
  tft.fillRect(0, 75, 320, 30, ILI9341_BLACK);
  tft.setTextSize(3);
  
  if(currentTimer <= 3) tft.setTextColor(ILI9341_RED);
  else tft.setTextColor(ILI9341_CYAN);
  
  // Center the countdown timer visually
  tft.setCursor(80, 80);
  tft.print("TIMER: "); 
  if (currentTimer < 10) tft.print("0"); // Leading zero
  tft.print(currentTimer); 
}