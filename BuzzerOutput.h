#ifndef BUZZER_OUTPUT_H
#define BUZZER_OUTPUT_H

#include <Arduino.h>

// Libraries Used:
    //<Adafruit_NeoPixel.h>
    //<DFRobotDFPlayerMini.h> 
    //<SPI.h>
    //<Adafruit_GFX.h>
    //<Adafruit_ILI9341.h>

// IMPORTANT, CHANGE THE NUM_LEDS (16) IF MORE LED STRIPS ARE ADDED OR REMOVED

#define NUM_LEDS    16      // Total series count of LED modules

// Hardware Pin Assignments (For Hardware Team) 
#define LED_PIN     6       
        //  for lcd display
#define TFT_CS      10      // CS
#define TFT_DC      9       // D/C
#define TFT_RST     8       // RESET
// MOSI (Data Out) = Pin 51
// MISO (Data In)  = Pin 50
// SCK (Clock)     = Pin 52

// DFPlayer RX connects to Mega TX1 (Pin 18). NOTE: might need to add a resistor in series
// DFPlayer TX connects to Mega RX1 (Pin 19).

// Use MicroSD card with FAT32 format
    // FILE NAME FOR AUDIO
    // 0001.mp3 : Floor Claimed / Buzzer Activation
    // 0002.mp3 : Positive Feedback / Correct Answer
    // 0003.mp3 : Negative Feedback / Locked Out / Timeout

//  Executes hardware setup (Display/LED) 
void initOutputs();   //Call once in setup().

//Processes visual animations.
void updateOutputs(); // Call continuously in loop().

// Void Trigger Settings

void triggerInitialSettings();            // Reverts to default standby: LEDs disabled, Display idle.
void triggerWhiteSetting();               // Proximity event: LEDs solid White, Display detects player.
void triggerBlueSetting();                // Standby configuration (Blue).
void triggerOrangeSetting();              // Standby configuration (Orange).
void triggerStartQuestion();              // Active question phase: Mutes mic, Display shows "Reading Question".
void triggerFloorClaimed();               // Primary event: Activates mic relay, starts LED animation, plays track 1.
void triggerCorrectAnswer(int newPoints); // Success: LEDs Green, plays track 2, updates scoring UI.
void triggerWrongOrTimeout();             // Fail/Timeout: Mutes mic, LEDs Red, plays track 3, displays lockout UI.

void updateLCDTimer(int remainingSeconds); // Refreshes the display timer parameters.
#endif