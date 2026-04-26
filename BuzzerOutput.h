#ifndef BUZZER_OUTPUT_H
#define BUZZER_OUTPUT_H

#include <Arduino.h>

//       BUZZER & HARDWARE OUTPUT MODULE

//Contains all the functions for OUTPUT modules
// the LEDs, LCD, Buzzer, and Contestant Mic.

// --- OUTPUT PIN DEFINITIONS ---
// (Important for hardware implementation)
#define LED_PIN     6       // Pin for the NeoPixel data line
#define NUM_LEDS    16      // Total number of LEDs on the strip
#define MIC_RELAY   7       // Pin controlling the Contestant Mic relay
#define BUZZER_PIN  8       // Pin for the passive buzzer

// --- CORE FUNCTIONS ---
void initOutputs();         // Call ONCE inside your main setup()
void updateOutputs();       // Call CONTINUOUSLY inside your main loop()

// --- STATE TRIGGER FUNCTIONS ---
// Call these whenever the game logic changes state to trigger the right hardware effects.

void triggerInitialSettings();            // Resets system: Mic OFF, LEDs OFF, LCD says "System Ready"
void triggerWhiteSetting();               // Sets LEDs to solid White, Mic OFF
void triggerBlueSetting();                // Sets LEDs to solid Blue, Mic OFF
void triggerOrangeSetting();              // Sets LEDs to solid Orange, Mic OFF
void triggerStartQuestion();              // Call when host reads: Locks out contestant Mic (OFF)
void triggerFloorClaimed();               // Call on buzz-in: Mic ON, plays beep, starts LED rainbow, LCD says "Answer Timer"
void triggerCorrectAnswer(int newPoints); // Call on correct answer: Mic OFF, LEDs Green, plays win sound, displays points
void triggerWrongOrTimeout();             // Call on wrong/timeout: Mic OFF, LEDs Red, plays error buzz, LCD says "Locked Out"
void updateLCDTimer(int remainingSeconds);// Updates the bottom row of the LCD with the countdown timer

#endif