#ifndef BUZZER_OUTPUT_H
#define BUZZER_OUTPUT_H

#include <Arduino.h>
#include <arduinomega/config.h>

// Libraries Used:
// <Adafruit_NeoPixel.h>
// <DFRobotDFPlayerMini.h> 
// <SPI.h>
// <Adafruit_GFX.h>
// <Adafruit_ILI9341.h>

/** Pin Assigments
 * // LED STRIP SETTINGS
// IMPORTANT: Change NUM_LEDS if more LED strips are added or removed (Increments of 8)
#define NUM_LEDS    16      // Total series count of LED modules per player
#define LED_PIN_1   6       // Pin for Player 1 LEDs
#define LED_PIN_2   5       // Pin for Player 2 LEDs (

// TFT LCD DISPLAY SETTINGS (Shared SPI Bus)
#define TFT_CS_1    10      // Chip Select for Player 1
#define TFT_CS_2    11      // Chip Select for Player 2 
#define TFT_DC      9       // Data/Command (Shared)
#define TFT_RST     8       // Hardware Reset (Shared)

// Common
// MOSI = Pin 51 | MISO = Pin 50 | SCK = Pin 52

// AUDIO SETTINGS (DFPLAYER MINI)
// Hardware Serial1: RX = Pin 18 | TX = Pin 19
 */

// Use MicroSD card with FAT32 format
// FILE NAME FOR AUDIO 
// 0001.mp3 : Floor Claimed / Buzzer Activation
// 0002.mp3 : Positive Feedback / Correct Answer
// 0003.mp3 : Negative Feedback / Locked Out / Timeout

// Executes hardware setup (Display/LED) 
void initOutputs();   // Call once in setup().

// Processes visual animations.
void updateOutputs(); // Call continuously in loop().

// Void Trigger Settings (playerIndex: 1 = P1, 2 = P2, 0 = Both Players)

void triggerInitialSettings(int playerIndex);  // Reverts to default standby: LEDs disabled, Display idle.
void triggerWhiteSetting(int playerIndex);     // Proximity event: LEDs solid White, Display detects player.
void triggerBlueSetting(int playerIndex);      // Standby configuration (Blue).
void triggerOrangeSetting(int playerIndex);    // Standby configuration (Orange).
void triggerStartQuestion(int playerIndex);    // Active question phase: Display shows "Reading Question".
void triggerFloorClaimed(int playerIndex);     // Primary event: Starts LED animation, plays track 1.
void triggerCorrectAnswer(int playerIndex, int newPoints); // Success: LEDs Green, plays track 2, updates scoring UI.
void triggerLockedOut(int playerIndex);   // Display 'Locked Out mode'
void triggerWrongOrTimeout(int playerIndex); // Display when wrong answer

void updateLCDTimer(int playerIndex, int remainingSeconds); // Refreshes the display timer parameters.

#endif