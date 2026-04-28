#include <Arduino.h>

#include "CoreSystem.h"
#include "BuzzerOutput.h"
#include "InputSystem.h"
#include "HostControl.h"

/* =========================================================
   BASIC CONFIG
   ========================================================= */

const int NUM_PLAYERS = 2;

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

int playerScore[NUM_PLAYERS] = {0};

int activePlayer  = -1;
int contestPlayer = -1;

bool contestActive = false;

/* =========================================================
   FUNCTION DECLARATIONS
   ========================================================= */

void updateCoreState(HostAction action);
void handleGlobalOverrides(HostAction action);
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

int getBuzzedPlayer();
int getContestPressedPlayer();

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

  int pressedContestPlayer = getContestPressedPlayer();

  if (pressedContestPlayer != -1 && currentState != STATE_CONTEST) {
    previousState = currentState;
    currentState = STATE_CONTEST;

    contestActive = true;
    contestPlayer = pressedContestPlayer;

    setPlayerContestVisuals(contestPlayer);
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
    triggerStartQuestion();
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

      // Future Output Team:
      // triggerCorrectAnswer(activePlayer, playerScore[activePlayer]);

      // Current Output Team:
      triggerCorrectAnswer(playerScore[activePlayer]);
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
    updateLCDTimer(mainTimeRemaining / 1000);
  }

  if (currentState == STATE_ANSWERING) {
    updateLCDTimer(answerTimeRemaining / 1000);
  }
}

/* =========================================================
   PLAYER VISUAL WRAPPERS
   ========================================================= */

void setPlayerAvailableVisuals(int playerIndex) {
  // Future:
  // triggerWhiteSetting(playerIndex);

  // Current Output Team:
  triggerWhiteSetting();
}

void setPlayerAbsentVisuals(int playerIndex) {
  // Future:
  // triggerInitialSettings(playerIndex);

  // Current Output Team:
  triggerInitialSettings();
}

void setPlayerLockedOutVisuals(int playerIndex) {
  // Future:
  // triggerWrongOrTimeout(playerIndex);

  // Current Output Team:
  triggerWrongOrTimeout();
}

void setPlayerTimedOutVisuals(int playerIndex) {
  // Future:
  // triggerOrangeSetting(playerIndex);

  // Current Output Team:
  triggerOrangeSetting();
}

void setPlayerBuzzedVisuals(int playerIndex) {
  // Future:
  // triggerFloorClaimed(playerIndex);

  // Current Output Team:
  triggerFloorClaimed();
}

void setPlayerContestVisuals(int playerIndex) {
  // Future:
  // triggerBlueSetting(playerIndex);

  // Current Output Team:
  triggerBlueSetting();
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
  }

  mainTimeRemaining = 0;
  answerTimeRemaining = 0;

  currentState = STATE_WAIT_FOR_PLAYERS;
  previousState = STATE_WAIT_FOR_PLAYERS;

  triggerInitialSettings();
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