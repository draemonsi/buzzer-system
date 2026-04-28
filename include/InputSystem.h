#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

// ================= SETUP & UPDATE =================
void setupInputSystem();
void updateInputSystem();

// ================= PLAYER INPUT =================

// Returns true if a player is physically present (ultrasonic)
bool isPlayerPresent(int playerIndex);

// Returns player index (0,1,...) if someone buzzed
// Returns -1 if none
int getBuzzedPlayerFromInput();

// Returns player index if contest button pressed
// Returns -1 if none
int getContestPressedPlayerFromInput();

#endif