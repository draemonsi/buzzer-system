#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

// Entry points for the entire system
void coreSetup();
void coreLoop();

bool allPlayersDetected();
bool allPlayersLockedOut();

void startMainTimer();
void updateMainTimer();

void startAnswerTimer();
void updateAnswerTimer();

void startNextRound();
void resetWholeSystem();

void setPlayerAvailableVisuals(int playerIndex);
void setPlayerAbsentVisuals(int playerIndex);
void setPlayerLockedOutVisuals(int playerIndex);
void setPlayerTimedOutVisuals(int playerIndex);
void setPlayerBuzzedVisuals(int playerIndex);
void setPlayerContestVisuals(int playerIndex);
void restorePlayerVisuals();

#endif