#include "arduinomega/HostControl.h"
#include "arduinomega/config.h"

#include <Arduino.h>

// --- MEGA PIN DEFINITIONS ---
#define BUZZER_P1_PIN 2
#define BUZZER_P2_PIN 3

// --- GAME STATE VARIABLES ---
// This boolean acts as a "lock" to prevent players from buzzing
// before the host clicks "START" on the ESP32 screen.
bool acceptBuzzers = false; 

void setup() {
  // 1. Initialize USB Serial for PC Debugging
  Serial.begin(9600);
  Serial.println("Arduino Mega Booting...");

  // 2. Initialize Hardware Serial1 for ESP32 Communication
  // Pin 18 (TX1) goes to ESP32 RX2 (16)
  // Pin 19 (RX1) goes to ESP32 TX2 (17)
  Serial1.begin(9600); 

  // 3. Setup Button Pins with Internal Pull-Ups
  pinMode(BUZZER_P1_PIN, INPUT_PULLUP);
  pinMode(BUZZER_P2_PIN, INPUT_PULLUP);

  Serial.println("Mega Game Controller Ready!");
  Serial.println("Waiting for Host ESP32 commands...");
}

void loop() {
  handleESP32Commands();
  checkPhysicalBuzzers();
}

// --- FUNCTION: READ ESP32 COMMANDS ---
void handleESP32Commands() {
  if (Serial1.available()) {
    // Read the incoming command from the ESP32
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim(); // Remove invisible newline/carriage return characters
    
    // Print to PC Serial Monitor for debugging
    Serial.println("ESP32 SENT: " + cmd);

    // Act on the specific commands
    if (cmd == "CMD:START") {
      acceptBuzzers = true; // Unlock the buzzers!
      Serial.println("SYSTEM: Buzzers Unlocked. Waiting for players...");
    } 
    else if (cmd == "CMD:TIMEOUT" || cmd == "CMD:RESET_ROUND" || cmd == "CMD:RESTART_GAME") {
      acceptBuzzers = false; // Lock the buzzers
      Serial.println("SYSTEM: Buzzers Locked (Round Reset/Timeout).");
    }
    else if (cmd == "CMD:CORRECT" || cmd == "CMD:WRONG") {
      acceptBuzzers = false; // Lock the buzzers after a ruling is made
      Serial.println("SYSTEM: Buzzers Locked (Waiting for next question).");
    }
  }
}

// --- FUNCTION: READ PLAYER BUZZERS ---
void checkPhysicalBuzzers() {
  // Only check the buttons if the host has started the timer
  if (acceptBuzzers) {
    
    // Check Player 1
    if (digitalRead(BUZZER_P1_PIN) == LOW) {
      Serial1.println("BUZ:1");       // Send the trigger to the ESP32
      Serial.println("SYSTEM: P1 BUZZED IN!"); // Log it to PC
      acceptBuzzers = false;          // Lock buzzers so P2 can't buzz simultaneously
      delay(300);                     // Debounce delay
    }
    
    // Check Player 2
    else if (digitalRead(BUZZER_P2_PIN) == LOW) {
      Serial1.println("BUZ:2");       // Send the trigger to the ESP32
      Serial.println("SYSTEM: P2 BUZZED IN!"); // Log it to PC
      acceptBuzzers = false;          // Lock buzzers so P1 can't buzz simultaneously
      delay(300);                     // Debounce delay
    }
    
  }
}