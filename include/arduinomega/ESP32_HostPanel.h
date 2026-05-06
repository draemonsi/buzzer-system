#ifndef ESP32_HOSTPANEL_H
#define ESP32_HOSTPANEL_H

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

// --- ESP32 TFT SPI PIN DEFINITIONS ---
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4

// --- HARDWARE SERIAL PINS (ESP32 to MEGA) ---
#define RX2_PIN  16
#define TX2_PIN  17

// --- PUBLIC FUNCTIONS ---
void setupHostPanel();
void loopHostPanel();

// --- INTERNAL HELPERS ---
void drawMainWindow();
void processTouch();
void handleIncomingData();
void updateStatusDisplay(String status, uint16_t color);
void updateScoreDisplay();
void updateSettingsDisplay();
void updateTimerDisplay();

#endif