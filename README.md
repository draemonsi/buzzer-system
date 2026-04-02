# buzzer-system
ECE21120 Final Project Design
# Buzzer System

## Overview
This project involves the design, development, and testing of a buzzer system intended for quiz or game-based applications. The system detects contestant inputs, determines the first valid response, and provides corresponding visual and audio feedback.

The project follows a structured approach starting from software simulation and progressing toward full hardware implementation.

## Objectives
- To design a functional buzzer input system  
- To implement an LED display system for visual feedback  
- To develop an audio/sound system for user feedback  
- To ensure correct system behavior through simulation prior to hardware integration  

## System Description

### 1. Buzzer Input System
- Accepts inputs from multiple contestants through push buttons  
- Detects and processes the first valid button press  
- Ensures reliable and accurate input detection  

### 2. LED Display System
- Provides visual indication of system status  
- Identifies the contestant who pressed first  
- Displays system responses during operation  

### 3. Audio / Sound System
- Generates sound signals for system events  
- Provides feedback such as buzzer activation or timeout  

## System Workflow
1. The system waits for contestant input  
2. A button press is detected  
3. The first valid input is identified  
4. The system outputs:
   - Visual feedback through LEDs  
   - Audio feedback through the sound system  
5. The system resets for the next round  

## Software Simulation
The system is initially validated through software simulation by testing each module individually and as an integrated system.

Simulation verifies that:
- Inputs are correctly detected  
- Outputs respond according to system logic  
- The system behaves correctly under different conditions  

## Implementation Scope
This repository contains:
- Source code for system logic and control  
- Simulation files and test cases  
- Documentation related to system design and behavior  
