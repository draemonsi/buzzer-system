#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

// Entry points for the entire system
void coreSetup();
void coreLoop();

void updateCoreState(HostAction action);
void handleGlobalOverrides(HostAction action);
void handlePhysicalTimeoutRequests();
void updatePlayerTimeoutReturnStatus();
void updateCoreOutputTriggers();

void handleWaitForPlayers(HostAction action);
void handlePreGameIdle(HostAction action);
void handleHostQuestion(HostAction action);
void handleBuzzerOpen(HostAction action);
void handleAnswering(HostAction action);
void handleMainTimeout(HostAction action);
void handleContest(HostAction action);
void handleRoundOver(HostAction action);

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