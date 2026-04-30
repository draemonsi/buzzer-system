#include <Arduino.h>

#include "CoreSystem.h"
#include "BuzzerOutput.h"
#include "InputSystem.h"
#include "HostControl.h"
#include "config.h"

/* =========================================================
   BASIC CONFIG
   ========================================================= */

unsigned long mainTimerDuration   = 60000;
unsigned long answerTimerDuration = 10000;

unsigned long mainTimerStart   = 0;
unsigned long answerTimerStart = 0;

unsigned long mainTimeRemaining   = 0;
unsigned long answerTimeRemaining = 0;

/* =========================================================
   SYSTEM STATES
   ========================================================= */

enum SystemState {
  STATE_WAIT_FOR_PLAYERS,
  STATE_PRE_GAME_IDLE,
  STATE_HOST_QUESTION,
  STATE_BUZZER_OPEN,
  STATE_ANSWERING,
  STATE_MAIN_TIMEOUT,
  STATE_CONTEST,
  STATE_ROUND_OVER
};

SystemState currentState  = STATE_WAIT_FOR_PLAYERS;
SystemState previousState = STATE_WAIT_FOR_PLAYERS;

/* =========================================================
   PLAYER DATA
   ========================================================= */

bool playerPresent[NUM_PLAYERS]   = {false};
bool playerLockedOut[NUM_PLAYERS] = {false};
bool playerTimedOut[NUM_PLAYERS]  = {false};
bool timeoutWaitingForLeave[NUM_PLAYERS]  = {false};
bool timeoutWaitingForReturn[NUM_PLAYERS] = {false};

int playerScore[NUM_PLAYERS] = {0};

int activePlayer  = -1;
int contestPlayer = -1;

bool contestActive = false;

/* =========================================================
   FUNCTION DECLARATIONS
   ========================================================= */

int getBuzzedPlayer();
int getContestPressedPlayer();
int getTimeoutPressedPlayer();
bool getForceStartPressed();

/* =========================================================
   CORE SETUP AND LOOP
   ========================================================= */

void coreSetup() {
  Serial.begin(9600);

  setupInputSystem();
  initOutputs();
  setupHostControl();

  resetWholeSystem();
}

void coreLoop() {
  updateInputSystem();

  for (int i = 0; i < NUM_PLAYERS; i++) {
    playerPresent[i] = isPlayerPresent(i);
  }

  updatePlayerTimeoutReturnStatus();
  handlePhysicalTimeoutRequests();

  HostAction hostAction = getHostAction();

  handleGlobalOverrides(hostAction);
  updateCoreState(hostAction);
  updateCoreOutputTriggers();

  updateOutputs();
}

/* =========================================================
   GLOBAL OVERRIDES
   ========================================================= */

void handleGlobalOverrides(HostAction action) {
  if (action == HOST_FORCE_RESET) {
    resetWholeSystem();
    return;
  }

  if (getForceStartPressed() && currentState == STATE_WAIT_FOR_PLAYERS) {
    currentState = STATE_PRE_GAME_IDLE;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      setPlayerAvailableVisuals(i);
    }

    return;
  }

  int pressedContestPlayer = getContestPressedPlayer();

  if (pressedContestPlayer != -1 && currentState != STATE_CONTEST) {
    previousState = currentState;
    currentState = STATE_CONTEST;

    contestActive = true;
    contestPlayer = pressedContestPlayer;

    setPlayerContestVisuals(contestPlayer);
  }
}

void handlePhysicalTimeoutRequests() {
  int timeoutPlayer = getTimeoutPressedPlayer();

  if (timeoutPlayer == -1) {
    return;
  }

  if (timeoutPlayer < 0 || timeoutPlayer >= NUM_PLAYERS) {
    return;
  }

  playerTimedOut[timeoutPlayer] = true;
  timeoutWaitingForLeave[timeoutPlayer] = true;
  timeoutWaitingForReturn[timeoutPlayer] = false;

  setPlayerTimedOutVisuals(timeoutPlayer);
}

void updatePlayerTimeoutReturnStatus() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (!playerTimedOut[i]) {
      continue;
    }

    if (timeoutWaitingForLeave[i] && !playerPresent[i]) {
      timeoutWaitingForLeave[i] = false;
      timeoutWaitingForReturn[i] = true;
    }

    if (timeoutWaitingForReturn[i] && playerPresent[i]) {
      playerTimedOut[i] = false;
      timeoutWaitingForLeave[i] = false;
      timeoutWaitingForReturn[i] = false;

      if (playerLockedOut[i]) {
        setPlayerLockedOutVisuals(i);
      } else {
        setPlayerAvailableVisuals(i);
      }
    }
  }
}

/* =========================================================
   CORE STATE MACHINE
   ========================================================= */

void updateCoreState(HostAction action) {
  switch (currentState) {
    case STATE_WAIT_FOR_PLAYERS:
      handleWaitForPlayers(action);
      break;

    case STATE_PRE_GAME_IDLE:
      handlePreGameIdle(action);
      break;

    case STATE_HOST_QUESTION:
      handleHostQuestion(action);
      break;

    case STATE_BUZZER_OPEN:
      handleBuzzerOpen(action);
      break;

    case STATE_ANSWERING:
      handleAnswering(action);
      break;

    case STATE_MAIN_TIMEOUT:
      handleMainTimeout(action);
      break;

    case STATE_CONTEST:
      handleContest(action);
      break;

    case STATE_ROUND_OVER:
      handleRoundOver(action);
      break;
  }
}

/* =========================================================
   STATE HANDLERS
   ========================================================= */

void handleWaitForPlayers(HostAction action) {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (playerPresent[i]) {
      setPlayerAvailableVisuals(i);
    } else {
      setPlayerAbsentVisuals(i);
    }
  }

  if (action == HOST_START && allPlayersDetected()) {
    currentState = STATE_PRE_GAME_IDLE;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      setPlayerAvailableVisuals(i);
    }
  }
}

void handlePreGameIdle(HostAction action) {
  if (action == HOST_READY) {
    currentState = STATE_HOST_QUESTION;

    // Output Team: mute contestant mics, keep host mic ON.
    triggerStartQuestion(action);
  }
}

void handleHostQuestion(HostAction action) {
  if (action == HOST_GO) {
    startMainTimer();
    currentState = STATE_BUZZER_OPEN;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      if (!playerLockedOut[i] && !playerTimedOut[i] && playerPresent[i]) {
        setPlayerAvailableVisuals(i);
      }
    }
  }
}

void handleBuzzerOpen(HostAction action) {
  updateMainTimer();

  if (mainTimeRemaining == 0) {
    currentState = STATE_MAIN_TIMEOUT;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      setPlayerTimedOutVisuals(i);
    }

    return;
  }

  restorePlayerVisuals();

  int buzzedPlayer = getBuzzedPlayer();

  if (buzzedPlayer != -1) {
    activePlayer = buzzedPlayer;
    startAnswerTimer();

    currentState = STATE_ANSWERING;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      if (i == activePlayer) {
        setPlayerBuzzedVisuals(i);
      }
    }
  }
}

void handleAnswering(HostAction action) {
  updateMainTimer();
  updateAnswerTimer();

  if (mainTimeRemaining == 0) {
    currentState = STATE_MAIN_TIMEOUT;

    for (int i = 0; i < NUM_PLAYERS; i++) {
      setPlayerTimedOutVisuals(i);
    }

    return;
  }

  if (action == HOST_CORRECT) {
    if (activePlayer != -1) {
      playerScore[activePlayer]++;
      triggerCorrectAnswer(playerScore[activePlayer], activePlayer);
    }

    currentState = STATE_ROUND_OVER;
    return;
  }

  if (action == HOST_WRONG || answerTimeRemaining == 0) {
    if (activePlayer != -1) {
      playerLockedOut[activePlayer] = true;
      setPlayerLockedOutVisuals(activePlayer);
    }

    activePlayer = -1;

    if (allPlayersLockedOut()) {
      currentState = STATE_ROUND_OVER;
    } else {
      currentState = STATE_BUZZER_OPEN;

      for (int i = 0; i < NUM_PLAYERS; i++) {
        if (!playerLockedOut[i] && !playerTimedOut[i] && playerPresent[i]) {
          setPlayerAvailableVisuals(i);
        }
      }
    }

    return;
  }
}

void handleMainTimeout(HostAction action) {
  if (action == HOST_NEXT_ROUND) {
    startNextRound();
  }
}

void handleContest(HostAction action) {
  if (action == HOST_CONTEST_RESOLVED) {
    contestActive = false;
    contestPlayer = -1;

    currentState = previousState;
    restorePlayerVisuals();
  }
}

void handleRoundOver(HostAction action) {
  if (action == HOST_NEXT_ROUND) {
    startNextRound();
  }
}

/* =========================================================
   CORE OUTPUT TRIGGERS
   ========================================================= */

void updateCoreOutputTriggers() {
  if (currentState == STATE_BUZZER_OPEN) {
    updateLCDTimer(-1, mainTimeRemaining / 1000);
  }

  if (currentState == STATE_ANSWERING) {
    updateLCDTimer(-1, answerTimeRemaining / 1000);
  }
}

/* =========================================================
   PLAYER VISUAL WRAPPERS
   ========================================================= */

void setPlayerAvailableVisuals(int playerIndex) {
  triggerWhiteSetting(playerIndex);
}

void setPlayerAbsentVisuals(int playerIndex) {
  triggerInitialSettings(playerIndex);
}

void setPlayerLockedOutVisuals(int playerIndex) {
  triggerWrongOrTimeout(playerIndex);
}

void setPlayerTimedOutVisuals(int playerIndex) {
  triggerOrangeSetting(playerIndex);
}

void setPlayerBuzzedVisuals(int playerIndex) {
  triggerFloorClaimed(playerIndex);
}

void setPlayerContestVisuals(int playerIndex) {
  triggerBlueSetting(playerIndex);
}

void restorePlayerVisuals() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (playerLockedOut[i]) {
      setPlayerLockedOutVisuals(i);
    } else if (playerTimedOut[i] || !playerPresent[i]) {
      setPlayerTimedOutVisuals(i);
    } else {
      setPlayerAvailableVisuals(i);
    }
  }
}

/* =========================================================
   CORE HELPERS
   ========================================================= */

bool allPlayersDetected() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (!playerPresent[i]) {
      return false;
    }
  }
  return true;
}

bool allPlayersLockedOut() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (!playerLockedOut[i] && !playerTimedOut[i] && playerPresent[i]) {
      return false;
    }
  }
  return true;
}

void startMainTimer() {
  mainTimerStart = millis();
  mainTimeRemaining = mainTimerDuration;
}

void updateMainTimer() {
  unsigned long elapsed = millis() - mainTimerStart;

  if (elapsed >= mainTimerDuration) {
    mainTimeRemaining = 0;
  } else {
    mainTimeRemaining = mainTimerDuration - elapsed;
  }
}

void startAnswerTimer() {
  answerTimerStart = millis();
  answerTimeRemaining = answerTimerDuration;
}

void updateAnswerTimer() {
  unsigned long elapsed = millis() - answerTimerStart;

  if (elapsed >= answerTimerDuration) {
    answerTimeRemaining = 0;
  } else {
    answerTimeRemaining = answerTimerDuration - elapsed;
  }
}

void startNextRound() {
  activePlayer = -1;

  for (int i = 0; i < NUM_PLAYERS; i++) {
    playerLockedOut[i] = false;
  }

  mainTimeRemaining = mainTimerDuration;
  answerTimeRemaining = answerTimerDuration;

  currentState = STATE_PRE_GAME_IDLE;

  restorePlayerVisuals();
}

void resetWholeSystem() {
  activePlayer = -1;
  contestPlayer = -1;
  contestActive = false;

  for (int i = 0; i < NUM_PLAYERS; i++) {
    playerScore[i] = 0;
    playerLockedOut[i] = false;
    playerTimedOut[i] = false;
    timeoutWaitingForLeave[i] = false;
    timeoutWaitingForReturn[i] = false;
  }

  mainTimeRemaining = 0;
  answerTimeRemaining = 0;

  currentState = STATE_WAIT_FOR_PLAYERS;
  previousState = STATE_WAIT_FOR_PLAYERS;

  triggerInitialSettings(-1);
}

/* =========================================================
   INPUT BRIDGES
   ========================================================= */

int getBuzzedPlayer() {
  return getBuzzedPlayerFromInput();
}

int getContestPressedPlayer() {
  return getContestPressedPlayerFromInput();
}

int getTimeoutPressedPlayer() {
  return getTimeoutPressedPlayerFromInput();
}

bool getForceStartPressed() {
  return isForceStartPressedFromInput();
}