/*
  BUZZER SYSTEM CORE BACKBONE

  Purpose:
  - Input Team, Output Team, and Host Team will plug their code
    sa mga marked sections.
*/

#include <Arduino.h>

/* =========================================================
   BASIC CONFIG
   ========================================================= */

const int NUM_PLAYERS = 2;

// Default timers, pwede palitan ng Host Team through TFT
unsigned long mainTimerDuration   = 60000;  // 60 seconds
unsigned long answerTimerDuration = 10000;  // 10 seconds

unsigned long mainTimerStart   = 0;
unsigned long answerTimerStart = 0;

unsigned long mainTimeRemaining   = 0;
unsigned long answerTimeRemaining = 0;

/* =========================================================
   SYSTEM STATES
   ========================================================= */

enum SystemState {
  STATE_WAIT_FOR_PLAYERS,   // waiting muna madetect lahat ng players
  STATE_PRE_GAME_IDLE,      // game started, pero hindi pa question phase
  STATE_HOST_QUESTION,      // host nagtatanong, buzzers locked
  STATE_BUZZER_OPEN,        // buzzers unlocked, main timer running
  STATE_ANSWERING,          // may nagbuzz, answer timer running
  STATE_MAIN_TIMEOUT,       // main timer expired
  STATE_CONTEST,            // contest/appeal mode
  STATE_ROUND_OVER          // round finished
};

SystemState currentState  = STATE_WAIT_FOR_PLAYERS;
SystemState previousState = STATE_WAIT_FOR_PLAYERS;

/* =========================================================
   HOST ACTIONS FROM TFT
   ========================================================= */

enum HostAction {
  HOST_NONE,

  HOST_START,              // start game after all players detected
  HOST_READY,              // host ready to ask question
  HOST_GO,                 // unlock buzzers and start timer

  HOST_CORRECT,            // active player's answer is correct
  HOST_WRONG,              // active player's answer is wrong

  HOST_NEXT_ROUND,         // proceed to next round
  HOST_FORCE_RESET,        // full reset, scores included

  HOST_TIMEOUT_PLAYER,     // player-specific timeout, optional
  HOST_CONTEST_RESOLVED,   // contest issue resolved

  HOST_SET_MAIN_TIMER,     // optional: from TFT
  HOST_SET_ANSWER_TIMER    // optional: from TFT
};

/* =========================================================
   PLAYER DATA
   ========================================================= */

bool playerPresent[NUM_PLAYERS]   = {false};
bool playerLockedOut[NUM_PLAYERS] = {false};
bool playerTimedOut[NUM_PLAYERS]  = {false};

int playerScore[NUM_PLAYERS] = {0};

int activePlayer  = -1;  // player currently answering
int contestPlayer = -1;  // player who pressed contest

bool contestActive = false;

/* =========================================================
   FUNCTION DECLARATIONS
   ========================================================= */

void updateInputs();
HostAction getHostAction();
void handleGlobalOverrides(HostAction action);
void updateCoreState(HostAction action);
void updateOutputs();

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

int getBuzzedPlayer();
int getContestPressedPlayer();

/* =========================================================
   SETUP
   ========================================================= */

void setup() {
  Serial.begin(9600);

  // ================= INPUT TEAM =================
  // Dito ilagay setup ng:
  // - ultrasonic sensors
  // - buzzer buttons
  // - contest buttons
  //
  // Example:
  // setupInputSystem();

  // ================= OUTPUT TEAM =================
  // Dito ilagay setup ng:
  // - chassis LEDs
  // - buzzer LEDs
  // - LCD displays
  // - sound effects
  // - mic relays
  //
  // Example:
  // setupOutputSystem();

  // ================= HOST TEAM =================
  // Dito ilagay setup ng:
  // - TFT touchscreen
  // - GUI screens
  // - timer settings
  //
  // Example:
  // setupHostDisplay();

  resetWholeSystem();
}

/* =========================================================
   MAIN LOOP
   ========================================================= */

void loop() {
  updateInputs();

  HostAction hostAction = getHostAction();

  handleGlobalOverrides(hostAction);

  updateCoreState(hostAction);

  updateOutputs();
}

/* =========================================================
   INPUT UPDATE
   ========================================================= */

void updateInputs() {
  // ================= INPUT TEAM =================
  // Dito i-update lagi yung inputs.
  //
  // Required updates:
  // - playerPresent[] from ultrasonic sensors
  // - player buzzer button status
  // - contest button status
  //
  // Example:
  // playerPresent[0] = readUltrasonicSensor(0);
  // playerPresent[1] = readUltrasonicSensor(1);
}

/* =========================================================
   HOST ACTION READER
   ========================================================= */

HostAction getHostAction() {
  // ================= HOST TEAM =================
  // Dito manggagaling lahat ng TFT touchscreen actions.
  //
  // Example:
  // if START button touched, return HOST_START;
  // if READY button touched, return HOST_READY;
  // if GO button touched, return HOST_GO;
  // if CORRECT button touched, return HOST_CORRECT;
  // if WRONG button touched, return HOST_WRONG;
  // if NEXT ROUND touched, return HOST_NEXT_ROUND;
  // if RESET touched, return HOST_FORCE_RESET;
  //
  // Timer setting example:
  // mainTimerDuration = selectedMainTimerFromTFT;
  // answerTimerDuration = selectedAnswerTimerFromTFT;

  return HOST_NONE;
}

/* =========================================================
   GLOBAL OVERRIDES
   These can happen kahit nasa anong state.
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

    // ================= OUTPUT TEAM =================
    // Gawing BLUE yung chassis LED ng nagcontest.
    // Example:
    // setPlayerLED(contestPlayer, BLUE);

    // ================= HOST TEAM =================
    // TFT should show:
    // "Player X Contest"
    // "Issue resolved?"
    // Button: RESOLVED
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
   STATE: WAIT FOR PLAYERS
   ========================================================= */

void handleWaitForPlayers(HostAction action) {
  // Bago magsimula:
  // - Host cannot start if hindi detected lahat ng players.
  // - LED OFF kapag walang player.
  // - LED WHITE kapag detected yung player.

  if (allPlayersDetected()) {
    // ================= OUTPUT TEAM =================
    // All detected chassis LEDs = WHITE.
    // Example:
    // setAllChassisLEDsWhite();

    // ================= HOST TEAM =================
    // Enable START button sa TFT.
    // Example:
    // showStartEnabled();
  } else {
    // ================= OUTPUT TEAM =================
    // Update LEDs based on ultrasonic presence.
    // Present = WHITE, not present = OFF.

    // ================= HOST TEAM =================
    // Disable START button.
    // Example:
    // showWaitingForPlayers();
  }

  if (action == HOST_START && allPlayersDetected()) {
    currentState = STATE_PRE_GAME_IDLE;

    // ================= OUTPUT TEAM =================
    // Game online na:
    // - chassis LEDs = WHITE
    // - buzzer LEDs = ON
    // - player mic relays = ON
    // - host mic relay = ON
    // - buzzers still locked
  }
}

/* =========================================================
   STATE: PRE GAME IDLE
   ========================================================= */

void handlePreGameIdle(HostAction action) {
  // Game started na.
  // No question yet.
  // Buzzers locked.
  // Player mics ON.
  // Host mic ON.

  if (action == HOST_READY) {
    currentState = STATE_HOST_QUESTION;

    // ================= OUTPUT TEAM =================
    // Host will ask question:
    // - contestant mics OFF
    // - host mic remains ON
    // - buzzers still locked
  }
}

/* =========================================================
   STATE: HOST QUESTION
   ========================================================= */

void handleHostQuestion(HostAction action) {
  // Host asking question.
  // Buzzers locked.
  // Contestant mics OFF.
  // Host mic ON.

  if (action == HOST_GO) {
    startMainTimer();
    currentState = STATE_BUZZER_OPEN;

    // ================= OUTPUT TEAM =================
    // GO phase:
    // - allowed buzzer LEDs ON
    // - LCD displays main timer
    // - sound effect: start/go beep
    // - buzzers unlocked by logic
  }
}

/* =========================================================
   STATE: BUZZER OPEN
   ========================================================= */

void handleBuzzerOpen(HostAction action) {
  // Buzzers unlocked.
  // Main timer running.
  // First valid player to buzz gets activePlayer.

  updateMainTimer();

  if (mainTimeRemaining == 0) {
    currentState = STATE_MAIN_TIMEOUT;

    // ================= OUTPUT TEAM =================
    // Main timeout:
    // - sound effect timeout
    // - buzzers locked
    // - display TIMEOUT
    return;
  }

  int buzzedPlayer = getBuzzedPlayer();

  if (buzzedPlayer != -1) {
    activePlayer = buzzedPlayer;
    startAnswerTimer();

    currentState = STATE_ANSWERING;

    // ================= OUTPUT TEAM =================
    // Active player:
    // - buzzer LED stays ON
    // - chassis LED = RAINBOW
    // - mic relay ON
    //
    // Other players:
    // - buzzer LEDs OFF
    // - chassis LEDs OFF/dim, except lockedout red or timeout orange
    //
    // LCD:
    // - show "BUZZED!" briefly
    // - then show answer timer
    //
    // Sound:
    // - play buzz sound
  }
}

/* =========================================================
   STATE: ANSWERING
   ========================================================= */

void handleAnswering(HostAction action) {
  // May nagbuzz.
  // Main timer still running.
  // Answer timer running.

  updateMainTimer();
  updateAnswerTimer();

  if (mainTimeRemaining == 0) {
    currentState = STATE_MAIN_TIMEOUT;
    return;
  }

  if (action == HOST_CORRECT) {
    playerScore[activePlayer]++;

    currentState = STATE_ROUND_OVER;

    // ================= OUTPUT TEAM =================
    // Correct answer:
    // - play correct sound
    // - update LCD score
    // - update TFT score
    // - celebration LED
    // - lock all buzzers
    return;
  }

  if (action == HOST_WRONG || answerTimeRemaining == 0) {
    playerLockedOut[activePlayer] = true;

    // ================= OUTPUT TEAM =================
    // Wrong or answer timeout:
    // - play wrong/timeout sound
    // - active player chassis LED = RED
    // - active player buzzer LED OFF
    // - active player mic OFF
    // - LCD: "Wrong" / "Locked Out"

    activePlayer = -1;

    if (allPlayersLockedOut()) {
      currentState = STATE_ROUND_OVER;
    } else {
      currentState = STATE_BUZZER_OPEN;

      // ================= OUTPUT TEAM =================
      // Remaining allowed players:
      // - buzzer LEDs ON
      // - chassis LEDs WHITE
      // Locked-out players remain RED.
    }

    return;
  }

  // ================= OUTPUT TEAM =================
  // Keep updating answer timer on LCD.
  // Keep active player's rainbow LED running.
}

/* =========================================================
   STATE: MAIN TIMEOUT
   ========================================================= */

void handleMainTimeout(HostAction action) {
  // Main timer expired.
  // Round basically ended due to time.

  if (action == HOST_NEXT_ROUND) {
    startNextRound();
  }

  // ================= HOST TEAM =================
  // TFT shows:
  // "MAIN TIMEOUT"
  // "NEXT ROUND"

  // ================= OUTPUT TEAM =================
  // Buzzers locked.
  // Timeout sound/effect.
}

/* =========================================================
   STATE: CONTEST
   ========================================================= */

void handleContest(HostAction action) {
  // Contest mode.
  // Game is temporarily held.
  // Contesting player's chassis LED = BLUE.

  if (action == HOST_CONTEST_RESOLVED) {
    contestActive = false;
    contestPlayer = -1;

    currentState = previousState;

    // ================= OUTPUT TEAM =================
    // Remove BLUE contest indicator.
    // Restore LEDs according to previous state.
  }
}

/* =========================================================
   STATE: ROUND OVER
   ========================================================= */

void handleRoundOver(HostAction action) {
  // Round is finished.
  // Scores stay.
  // Waiting for next round.

  if (action == HOST_NEXT_ROUND) {
    startNextRound();
  }

  // ================= HOST TEAM =================
  // Show scores and NEXT ROUND button.

  // ================= OUTPUT TEAM =================
  // Show score on LCD.
  // Lock all buzzers.
}

/* =========================================================
   CORE HELPER FUNCTIONS
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
    if (!playerLockedOut[i] && !playerTimedOut[i]) {
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

    // Keep scores.
    // Keep player presence.
    // Optional: clear playerTimedOut only if your team wants.
  }

  mainTimeRemaining = mainTimerDuration;
  answerTimeRemaining = answerTimerDuration;

  currentState = STATE_PRE_GAME_IDLE;

  // ================= OUTPUT TEAM =================
  // Reset round outputs:
  // - chassis LEDs white if present
  // - lockedout cleared
  // - buzzers locked
  // - buzzer LEDs ON
  // - player mics ON
  // - LCD shows score/standby
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

  // ================= OUTPUT TEAM =================
  // Full reset:
  // - all LEDs OFF
  // - all buzzer LEDs OFF
  // - all mics OFF
  // - LCD waiting screen
  // - reset sound optional

  // ================= HOST TEAM =================
  // TFT back to waiting/player detection screen.
}

/* =========================================================
   PLACEHOLDER FUNCTIONS FOR OTHER TEAMS
   Other teams will replace these.
   ========================================================= */

int getBuzzedPlayer() {
  // ================= INPUT TEAM =================
  // Return player index kung may valid buzz.
  // Return -1 kung wala.
  //
  // Dapat i-check:
  // - player is present
  // - player is not locked out
  // - player is not timed out
  // - buzzer button was pressed
  //
  // Example:
  // if player 1 pressed, return 0;
  // if player 2 pressed, return 1;

  return -1;
}

int getContestPressedPlayer() {
  // ================= INPUT TEAM =================
  // Return player index kung may nagpress ng contest button.
  // Return -1 kung wala.
  //
  // Example:
  // if player 1 contest pressed, return 0;
  // if player 2 contest pressed, return 1;

  return -1;
}

void updateOutputs() {
  // ================= OUTPUT TEAM =================
  // Dito aapply outputs based on:
  // - currentState
  // - activePlayer
  // - playerScore[]
  // - playerLockedOut[]
  // - playerTimedOut[]
  // - playerPresent[]
  // - mainTimeRemaining
  // - answerTimeRemaining
  //
  // Examples:
  //
  // if currentState == STATE_WAIT_FOR_PLAYERS:
  //   detected player = WHITE LED
  //   not detected = OFF
  //
  // if currentState == STATE_BUZZER_OPEN:
  //   available players = WHITE
  //   locked out players = RED
  //   timed out players = ORANGE
  //
  // if currentState == STATE_ANSWERING:
  //   active player = RAINBOW
  //   show answer timer on LCD
  //
  // if currentState == STATE_CONTEST:
  //   contest player = BLUE
}