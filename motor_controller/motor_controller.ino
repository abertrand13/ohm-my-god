//////////////////////////
//                      //
// ME 210 FINAL PROJECT //
// -------------------- //
// Chris Kimes          //
// Eni Asebiomo         //
// Cuthbert Sun         //
// Alex Bertrand        //
//                      //
//////////////////////////

/*---------------Includes-----------------------------------*/

#include <Timers.h>
#include <Signals.h>
#include "Motors.h"

/*---------------Module Defines-----------------------------*/

#define SERIAL_DEBUG 0

// Pinout

// Limit Switches
#define PIN_LIMIT_BL A0           // Back limit switch on left side
#define PIN_LIMIT_FL A1           // Front limit switch on left side
#define PIN_LIMIT_LF A2           // Left limit switch on front side
#define PIN_LIMIT_RF A3           // Right limit switch on front side

// Tape sensor
#define PIN_TAPE A4               // Tape sensor

// Timers
#define TMR_ALIGN 1               // Timer to rotate away from IR sensor
#define TMR_ALIGN_VAL 700         // Time to run timer for
#define TMR_RETURN_LEFT 7         // Timer to run along the wall for (minimize drift)
#define TMR_RETURN_LEFT_VAL 1200  // Time to run the timer for (constant, gets multiplied)
#define TMR_RETURN 2              // Timer to backup off wall, to get to safe space
#define TMR_RETURN_VAL 900        // Time to run return timer for
#define TMR_REFILL 3              // Timer to pause for refilling
#define TMR_REFILL_VAL 5000       // Time to run refill timer for
#define TMR_STRAFE_RIGHT 5        // Timer for avoiding balls
#define TMR_STRAFE_RIGHT_VAL 1300 // Time to run strafe timer for
#define TMR_DETECTTAPE 6          // Timer to wait to track tape after hitting the front wall
#define TMR_DETECTTAPE_VAL 500    // Time to run detect tape timer for
#define TMR_ROLLBACK 8            // Timer to roll back into safe space
#define TMR_ROLLBACK_VAL 200      // 
#define TMR_TIMEOUT 9             // Timer for if we get f**ked by balls on the wall
#define TMR_TIMEOUT_VAL 3000      // If a transition hasn't completed in 3 seconds that's bad
#define TMR_JITTER 4              // Make sure to drop the bass/tape sensor
#define TMR_JITTER_VAL 300

#define TIMER_0 0
#define ONE_SEC 1000

#define TAPE_THRESHOLD 500

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);

// Logic functions
bool checkIRAlign(void);
bool checkBackLeftLimitSwitch(void);
bool checkFrontLeftLimitSwitch(void);
bool checkLeftLimitSwitchesAligned(void);
bool checkFrontLimitSwitchesAligned(void);
bool checkTape(void);

// Event handlers
void handleIRAlign(void);
void handleTurnTimerExpired(void);
void handleFrontContact(void);
void handleBackContact(void);
void handleLeftLimitSwitchesAligned(void);
void handleFrontLimitSwitchesAligned(void);
void jitter(void);
void correctLimitSwitches(void);
void handleAlignmentTape(void);
void handleTape(void);
void startReturningLeft(void);
void handleReturnedLeft(void);
void handleReturnTimerExpired(void);
void handleTapeOnBackup(void);
void handleRolledBack(void);
void handleRefillTimerExpired(void);
void handleNextGoal(void);
void handleStoppage(void);
void setDestination(void);
void setupSensorPins(void);

// Debugging
void setHigh(void);
void setLow(void);
void checkOffHack(void);
void handchk(void);
void handchk2(void);

enum globalState {
  ALIGN_IR,       	// Get initial bearings with IR
  ALIGN_TURN,     	// Turn to face loosely right
  ALIGN_LEFT,     	// Move to hug the left wall
  ALIGN_LEFT_FRONT, // Switch to move only front wheel
  ALIGN_LEFT_BACK, 	// Switch to move only back wheel
  JITTER,           // to make sure we drop the tape sensor
  FIND_TAPE,        // Find the tape moving out of the safe space
  STRAFE_RIGHT,     // To avoid corner balls
  ALIGN_FRONT,    	// Move to hug the front wall
  WAIT4DEST,   		  // Waiting for 'Next Goal' signal after initial alignment @TD: @Q: do we need multiple different wait states? @A: Don't think so.
  MOVE2LEFT,    	  // Move to face the left goal
  MOVE2MID,     	  // Move to face the middle goal
  MOVE2RIGHT,     	// Move to face the right goal
  RETURN_LEFT_TIME, // Move left to return to *almost* the safe space
  RETURN_OFFWALL,   // Backup off wall 
  RETURN_LEFT,    	// Move left to return to safe space
  RETURN_BACK,    	// Move back to return to safe space
  ROLLBACK,         // Because I can't get hardBrake to work RIP
  REFILLING,        // Pause in the safe space for refill
  STOPPED           // for shutting down at the end of the match
};

/*---------------Module Variables---------------------------*/
enum globalState state;
bool tape;
bool onTape;
enum signal inputSignal;
enum Location location;
enum Location destination;
bool detectTape;
int jitters;

/*---------------Main Functions-----------------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();
  
  // Initial var setup
  state = ALIGN_IR;
  turnCCW(100); 

  location = REFILL;
  onTape = false;
  detectTape = false;
  inputSignal = NONE;
  jitters = 0;
}

void loop() { 
  checkEvents();
  applyMotorSettings();
}

/*---------------Event Detection Functions------------------*/

/******************************************************************************
  Function:    checkEvents
  Contents:    Checks both globally significant and state specific events,
               and calls the appropriate handlers.
  Parameters:  None
  Returns:     None
  Notes:       None
******************************************************************************/

void checkEvents() {
  inputSignal = receiveSignal(SERIAL_DEBUG);

  if(inputSignal == STOPAF) {
    state = STOPPED;
    handleStoppage();
  }

  switch(state) {
    case ALIGN_IR:
      if (checkIRAlign()) {
        handleIRAlign();
      }
      break;
    
    case ALIGN_TURN:
      if (TMRArd_IsTimerExpired(TMR_ALIGN)) { handleTurnTimerExpired(); }
      break;

    case ALIGN_LEFT: {
	    bool frontContact = checkFrontLeftLimitSwitch();
	    bool backContact = checkBackLeftLimitSwitch();
      if(checkLeftLimitSwitchesAligned()) { handleLeftLimitSwitchesAligned(); }
	    else if(frontContact) { handleFrontContact(); }
	    else if(backContact) { handleBackContact(); }
      break; } // braces for scoping

	  case ALIGN_LEFT_FRONT:
      if(checkLeftLimitSwitchesAligned()) { handleLeftLimitSwitchesAligned(); }
      else {
      state = ALIGN_LEFT;
      moveLeft(100);
      }
      break;

    case ALIGN_LEFT_BACK:
      if(checkLeftLimitSwitchesAligned()) { handleLeftLimitSwitchesAligned(); }
      else {
      state = ALIGN_LEFT;
      moveLeft(100);
      }
      break;

    case JITTER:
      jitter(); 
      break;

    case FIND_TAPE:
      if(checkTape()) { handleAlignmentTape(); }
      break;

    case STRAFE_RIGHT:
      finishStrafing();
      break;
    
    case ALIGN_FRONT:
      if(checkFrontLimitSwitchesAligned()) { handleFrontLimitSwitchesAligned(); }
      if(TMRArd_IsTimerExpired(TMR_TIMEOUT) == TMRArd_EXPIRED) { 
        TMRArd_ClearTimerExpired(TMR_TIMEOUT);
        handleFrontLimitSwitchesAligned(); // faking it
      }
      break;

   	/*--------- END ALIGNMENT STATES ---------*/ 
	
    case WAIT4DEST:
        handleNextGoal();
        break;

    case MOVE2LEFT:
      correctLimitSwitches();
      if(TMRArd_IsTimerExpired(TMR_DETECTTAPE)) {
        detectTape = true;
        TMRArd_ClearTimerExpired(TMR_DETECTTAPE);
        }
      if(checkTape() && detectTape) { 
        handleTape(); 
      }
      break;

    case MOVE2MID:
      correctLimitSwitches();
      if(checkTape() ) handleTape();
      break;    
    
    case MOVE2RIGHT:
      correctLimitSwitches();
      if(checkTape()) handleTape();
      break;

    case RETURN_LEFT_TIME:
      correctLimitSwitches();
      if(TMRArd_IsTimerExpired(TMR_RETURN_LEFT) == TMRArd_EXPIRED) { handleFirstReturnComplete(); }
      break;
    
    case RETURN_OFFWALL:
      if(TMRArd_IsTimerExpired(TMR_RETURN) == TMRArd_EXPIRED) { startReturningLeft(); }
      break;
       
    case RETURN_LEFT: 
      if(checkLeftLimitSwitchesAligned()) { handleReturnedLeft(); }
      if(TMRArd_IsTimerExpired(TMR_TIMEOUT)) {
        TMRArd_ClearTimerExpired(TMR_TIMEOUT);
        handleReturnedLeft();
      }
      break;

    case RETURN_BACK:
      if(checkTape()) { handleTapeOnBackup(); }
      break;

    case ROLLBACK:
      if(TMRArd_IsTimerExpired(TMR_ROLLBACK) == TMRArd_EXPIRED) { handleRolledBack(); }
      break;

    case REFILLING:
      if(TMRArd_IsTimerExpired(TMR_REFILL) == TMRArd_EXPIRED) { handleRefillTimerExpired(); }
      break;
  }   
}

/******************************************************************************
  Function:    checkIRAlign
  Contents:    Checks to see whether or not the alignment IR in the safe
               space has come 'into focus'
  Parameters:  None
  Returns:     Value of the IR detection circuit, {true, false}.
  Notes:       None
******************************************************************************/

bool checkIRAlign() {
  return inputSignal == FOUND_IR;
}

/******************************************************************************
  Function:    checkLeftLimitSwitchesAlign (and friends because i'm lazy)
  Contents:    Checks to see whether or not we have made contact with both
               limit switches on the left side of the robot. The assumption
               is that once we have, we are flush with the left wall of
               the echo chamber.
  Parameters:  None
  Returns:     Whether both limit switches have been pressed, {true, false}.
  Notes:       Assumes that limit switches have been wired in the
               'Normally Open' state.
******************************************************************************/

bool checkBackLeftLimitSwitch() {
  return digitalRead(PIN_LIMIT_BL);
}

bool checkFrontLeftLimitSwitch() {
  return digitalRead(PIN_LIMIT_FL);
}

bool checkLeftLimitSwitchesAligned() {
  return digitalRead(PIN_LIMIT_BL) & digitalRead(PIN_LIMIT_FL);
}

/******************************************************************************
  Function:    checkFrontLimitSwitchesAlign
  Contents:    Checks to see whether or not we have made contact with both
               limit switches on the front side of the robot. The assumption
               is that once we have, we are flush with the front wall of
               the echo chamber.
  Parameters:  None
  Returns:     Whether both limit switches have been pressed, {true, false}.
  Notes:       Assumes that limit switches have been wired in the
               'Normally Open' state.
******************************************************************************/

bool checkFrontLimitSwitchesAligned() {
  return analogRead(PIN_LIMIT_LF) & analogRead(PIN_LIMIT_RF);
}

/******************************************************************************
  Function:    checkTape
  Contents:    Checks to see whether or not we have hit a piece of tape on
               the echo chamber floor
  Parameters:  None
  Returns:     Boolean
  Notes:       Keeps track of whether or not we are currently on the tape,
               and only returns true if we go off->on.
               We may have to add some hysteresis on the sensor itself.
******************************************************************************/

bool checkTape() {
  tape = analogRead(PIN_TAPE) > TAPE_THRESHOLD;
  if(tape && !onTape) {
    onTape = true;
    return true;
  } else {
    onTape = tape;
    return false;
  }
}


/*---------------Event Handler Functions--------------------*/

void handleIRAlign() {
  state = ALIGN_TURN; 
  TMRArd_InitTimer(TMR_ALIGN, TMR_ALIGN_VAL); 
  turnCW(100);
}

void handleTurnTimerExpired() {
  state = ALIGN_LEFT;
  stopDriveMotors();
  TMRArd_ClearTimerExpired(TMR_ALIGN);
  // move left toward wall
  moveLeft(100);
}

void handleLeftLimitSwitchesAligned() {
  state = JITTER;
  TMRArd_InitTimer(TMR_JITTER, TMR_JITTER_VAL);
  stopDriveMotors();
  moveBack(127);
}

void handleFrontContact() {
  state = ALIGN_LEFT_BACK;
  stopDriveMotors();
  setMotorSpeed(MBACK, -100);
  setMotorSpeed(MLEFT, 75);
}

void handleBackContact() {
  state = ALIGN_LEFT_FRONT;
  stopDriveMotors();
  setMotorSpeed(MFRONT, -100);
  setMotorSpeed(MLEFT, -75);
}

void jitter() { 
  if(TMRArd_IsTimerExpired(TMR_JITTER) == TMRArd_EXPIRED) {
    jitters++;
    TMRArd_ClearTimerExpired(TMR_JITTER); 
    if(jitters == 2) {
      state = FIND_TAPE;
      stopDriveMotors();
      moveForward(100);
    } else {
      flipMotorDirection(MLEFT);
      flipMotorDirection(MRIGHT);
      TMRArd_InitTimer(TMR_JITTER, TMR_JITTER_VAL);
    }
  }
}

void handleFrontLimitSwitchesAligned() {
  stopDriveMotors();
  sendSignal(ALIGNED);
  state = WAIT4DEST;

  TMRArd_InitTimer(TMR_DETECTTAPE, TMR_DETECTTAPE_VAL);
}

void correctLimitSwitches() {
  setMotorSpeed(MRIGHT, digitalRead(PIN_LIMIT_RF) ? 0 : 50);
  setMotorSpeed(MLEFT, digitalRead(PIN_LIMIT_LF) ? 0 : 50);
}

void handleTape() { 
  // Operating on the assumption that we never skip goals...
  location = destination;
  
  // Indicate that we're ready to fire and then wait for further instructions
  stopDriveMotors();
  sendSignal(READY2FIRE);
  state = WAIT4DEST;
}

void handleAlignmentTape() {
  state = STRAFE_RIGHT;
  TMRArd_InitTimer(TMR_STRAFE_RIGHT, TMR_STRAFE_RIGHT_VAL);
  hardBrake(); 
  moveRight(100);
}

void finishStrafing() {
  if(TMRArd_IsTimerExpired(TMR_STRAFE_RIGHT)) {
    state = ALIGN_FRONT;
    stopDriveMotors();
    moveForward(100);
    TMRArd_InitTimer(TMR_TIMEOUT, TMR_TIMEOUT_VAL);
  }
}

void startReturningLeft() {
  state = RETURN_LEFT;
  TMRArd_InitTimer(TMR_TIMEOUT, TMR_TIMEOUT_VAL);
  stopDriveMotors();
  moveLeft(100);
}

void handleFirstReturnComplete() {
  // for when we finish our timed move left and we're almost at the safe space
  state = RETURN_OFFWALL;
  TMRArd_ClearTimerExpired(TMR_RETURN_LEFT);
  TMRArd_InitTimer(TMR_RETURN, TMR_RETURN_VAL);
  stopDriveMotors();
  moveBack(100);
}

void handleReturnedLeft() {
  state = RETURN_BACK;
  stopDriveMotors();
  moveBack(100);
  TMRArd_InitTimer(TMR_RETURN, TMR_RETURN_VAL);
}

void handleTapeOnBackup() {
  stopDriveMotors();
  moveForward(100);
  state = ROLLBACK;
  TMRArd_InitTimer(TMR_ROLLBACK, TMR_ROLLBACK_VAL);
}

void handleRolledBack() {
  stopDriveMotors();
  state = REFILLING;
  location = REFILL;
  TMRArd_ClearTimerExpired(TMR_ROLLBACK);
  TMRArd_InitTimer(TMR_REFILL, TMR_REFILL_VAL);
}

void handleRefillTimerExpired() {
  state = FIND_TAPE;
  setHigh();
  sendSignal(REFILL_DONE);
  TMRArd_ClearTimerExpired(TMR_REFILL);
  moveForward(100);
}

void handleNextGoal() { 
  
  switch(inputSignal) {
    case NEXT_LEFT:  
      state = MOVE2LEFT;
	    destination = GOAL_LEFT;
      setDestination();
      break;

    case NEXT_MID:
      state = MOVE2MID;
	    destination = GOAL_MID;
      setDestination();
      break;

    case NEXT_RIGHT:
      state = MOVE2RIGHT;
	    destination = GOAL_RIGHT;
      setDestination();
      break;

    case NEXT_REFILL: 
      state = RETURN_LEFT_TIME;
      TMRArd_InitTimer(TMR_RETURN_LEFT, TMR_RETURN_LEFT_VAL * location + ((location-1) ? 2000 : 0));
	    destination = REFILL;
      setDestination();
      break;
  }
}

void setDestination(void) {
  int loc = location;
  int dest = destination;

  if(loc < dest) {
    moveRight(100);
  } else if(loc > dest) {
    moveLeft(100);
  }
}

void setupPins() {
  pinMode(PIN_LIMIT_BL, INPUT);
  pinMode(PIN_LIMIT_FL, INPUT);
  pinMode(PIN_LIMIT_LF, INPUT);
  pinMode(PIN_LIMIT_RF, INPUT);
  pinMode(PIN_TAPE,     INPUT);

  // TX/RX 
  pinMode(0, INPUT);
  pinMode(1, OUTPUT);
}

void handleStoppage() {
  state = STOPPED; 
  stopDriveMotors();
}

void setHigh(void) {
    digitalWrite(A5, HIGH);
}

void setLow(void) {
    digitalWrite(A5, LOW);
}
