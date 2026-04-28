#ifndef HOST_CONTROL_H
#define HOST_CONTROL_H

// ================= HOST ACTION ENUM =================
enum HostAction {
  HOST_NONE,

  HOST_START,
  HOST_READY,
  HOST_GO,

  HOST_CORRECT,
  HOST_WRONG,

  HOST_NEXT_ROUND,
  HOST_FORCE_RESET,

  HOST_TIMEOUT_PLAYER,
  HOST_CONTEST_RESOLVED,

  HOST_SET_MAIN_TIMER,
  HOST_SET_ANSWER_TIMER
};

// ================= HOST FUNCTIONS =================

// Setup TFT + GUI
void setupHostControl();

// Read touchscreen actions
HostAction getHostAction();

#endif