#ifndef BUZZER_OUTPUT_H
#define BUZZER_OUTPUT_H

#include <Arduino.h>

// ================= HARDWARE CONFIG =================
#define NUM_LEDS    16
#define LED_PIN     6

#define TFT_CS      10
#define TFT_DC      9
#define TFT_RST     8

// ================= CORE OUTPUT =================

// Initialize all output hardware
void initOutputs();

// Continuous updates (animations like rainbow)
void updateOutputs();

// ================= SYSTEM STATES =================

// Reset / standby
void triggerInitialSettings();

// Player detected / ready
void triggerWhiteSetting();

// Contest mode (blue)
void triggerBlueSetting();

// Timeout / inactive (orange)
void triggerOrangeSetting();

// Host asking question
void triggerStartQuestion();

// Player buzzed (rainbow + sound)
void triggerFloorClaimed();

// Correct answer (green + score)
void triggerCorrectAnswer(int newPoints);

// Wrong / locked out (red)
void triggerWrongOrTimeout();

// Update timer display
void updateLCDTimer(int remainingSeconds);

#endif