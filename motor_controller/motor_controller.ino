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

// Pinout

// Limit Switches
#define PIN_LIMIT_BL A0     // Back limit switch on left side
#define PIN_LIMIT_FL A1     // Front limit switch on left side
#define PIN_LIMIT_LF A2     // Left limit switch on front side
#define PIN_LIMIT_RF A3     // Right limit switch on front side

// IR sensor @Q @TD I think this should this be removed from this 
// file bc sensing is happening in other arduino
#define PIN_IR_ALIGN A4     // IR sensor to align in safe space

// Tape sensor
#define PIN_TAPE A5         // Tape sensor

// Timers
#define TMR_ALIGN 1         // Timer to rotate away from IR sensor
#define TMR_ALIGN_VAL 250   // Time to run timer for
#define TMR_RETURN 2        // Timer to get to safe space
#define TMR_RETURN_VAL 1000 // Time to run return timer for
#define TMR_REFILL 3        // Timer to pause for refilling
#define TMR_REFILL_VAL 1500 // Time to run refill timer for

#define TIMER_0 0
#define ONE_SEC 1000

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);

// Logic functions
bool checkIRAlign(void);
bool checkBackLeftLimitSwitch(void);
bool checkFrontLeftLimitSwitch(void);
bool checkLeftLimitSwitchesAligned(void);
bool checkFrontLimitSwitchesAligned(void);
bool checkTape(void);

/* Signal Functions
// now pulled out into separate library
void updateSignal(void);
void sendSignal(char signal); */ 

// Event handlers
void handleIRAlign(void);
void handleTurnTimerExpired(void);
void handleFrontContact(void);
void handleBackContact(void);
void handleLeftLimitSwitchesAligned(void);
void handleFrontLimitSwitchesAligned(void);
void correctLimitSwitches(void);
void handleTape(void);
void handleReturnedLeft(void);
void handleReturnTimerExpired(void);
void handleRefillTimerExpired(void);
void handleNextGoal(void);
void setupSensorPins(void);

enum globalState {
  ALIGN_IR,       // Get initial bearings with IR
  ALIGN_TURN,     // Turn to face loosely right
  ALIGN_LEFT,     // Move to hug the left wall
  ALIGN_LEFT_FRONT, // Switch to move only front wheel
  ALIGN_LEFT_BACK, // Switch to move only back wheel
  ALIGN_FRONT,    // Move to hug the front wall
  WAIT4DEST,   // Waiting for 'Next Goal' signal after initial alignment @TD: @Q: do we need multiple different wait states? @A: Don't think so.
  MOVE2LEFT_1,    // Move to face the left goal
  SHOOT_LEFT_1,   // Shoot on the left goal
  MOVE2MID_1,     // Move to face the middle goal
  SHOOT_MID_1,    // Shoot on the middle goal
  MOVE2RIGHT,     // Move to face the right goal
  SHOOT_RIGHT,    // Shoot on the right goal
  MOVE2MID_2,     // Move to face the middle goal @Q: what's the difference btween this and MOVE2MID_1?
  SHOOT_MID_2,    // Shoot on the middle goal
  MOVE2LEFT_2,    // Move to face the left goal
  SHOOT_LEFT_2,   // Shoot on the left goal
  MOVE2RIGHT_1,   // Move to face the right goal
  RETURN_LEFT,    // Move left to return to safe space
  RETURN_BACK,    // Move back to return to safe space
  REFILL          // Pause in the safe space for refill
};

/*---------------Module Variables---------------------------*/
enum globalState state;
bool onTape;
enum signal inputSignal;

/*---------------Main Functions-----------------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();
  
  pinMode(A5, OUTPUT);
  delay(1000);
  digitalWrite(A5, HIGH);
  delay(500);
  digitalWrite(A5, LOW);

  // setMotorSpeed(MLEFT, 100);
  // setMotorSpeed(MRIGHT, 100);
  TMRArd_InitTimer(TIMER_0, ONE_SEC);
  state = ALIGN_IR;
}

void loop() { 
  // control routine (temporary)
  // to operate, just enter commands in the serial port
  // eg:
  // l127 sets left motor to full forward speed
  // l-127 sets left motor to full reverse speed
  // f127 set front motor to full forward speed
  // b-128 reverses the back motor, whether it's going forward or backward
  // if(Serial.available()) {
  //   char motor = Serial.read();
  //   enum motorID select;
  //   if(motor == 'l') {
  //     select = MLEFT;
  //   } else if(motor == 'r'){
  //     select = MRIGHT;
  //   } else if(motor == 'b'){
  //     select = MBACK;
  //   } else if(motor == 'f'){
  //     select = MFRONT;
  //   }
  //   int x = Serial.parseInt();
  //   Serial.println(x);

  //   setMotorSpeed(select, char(x));

  //   // if(x != -128) {
  //   //   setMotorSpeed(select, char(x));
  //   // } else {
  //   //   flipMotorDirection(select);
  //   // }
  //   Serial.read(); //newline @Q: Why is this here?
  // }
  
  checkEvents();

  applyMotorSettings();

  // For Debugging * NOTE will interrupt Serial Comms
  // if(TMRArd_IsTimerExpired(TIMER_0)) printStates();    
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
  inputSignal = receiveSignal();

  switch(state) {
    case ALIGN_IR:
      if (checkIRAlign()) handleIRAlign();
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

    case ALIGN_FRONT:
      if(checkFrontLimitSwitchesAligned()) { handleFrontLimitSwitchesAligned(); }
      break;

    case WAIT4DEST:
      handleNextGoal();
      break;

    case MOVE2LEFT_1:
      correctLimitSwitches();
      break;

    case MOVE2MID_1: 
      break;    
    case MOVE2RIGHT:
      break;
    case MOVE2MID_2:
      break;
    case MOVE2LEFT_2:
      if(checkTape()) { handleTape(); }
      break;

    case RETURN_LEFT:
      if(checkLeftLimitSwitchesAligned()) { handleReturnedLeft(); }
      break;

    case RETURN_BACK:
      if(TMRArd_IsTimerExpired(TMR_RETURN)) { handleReturnTimerExpired(); }
      break;

    case REFILL:
      if(TMRArd_IsTimerExpired(TMR_REFILL)) { handleRefillTimerExpired(); }
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
  return inputSignal == ALIGN_IR;
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
  bool tape = digitalRead(PIN_TAPE);
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
  // state = ALIGN_TURN; //commented for serial comms testing - uncomment when done
  
  TMRArd_InitTimer(TMR_ALIGN, TMR_ALIGN_VAL); 
  Serial.println("YEET");
}

void handleTurnTimerExpired() {
  state = ALIGN_LEFT;
  TMRArd_ClearTimerExpired(TMR_ALIGN);
  // move left toward wall
  moveLeft(100);
}

void handleLeftLimitSwitchesAligned() {
  state = ALIGN_FRONT;
  stopDriveMotors();
  moveForward(100);
}

void handleFrontContact() {
  state = ALIGN_LEFT_BACK;
  stopDriveMotors();
  setMotorSpeed(MBACK, -50);
}

void handleBackContact() {
  state = ALIGN_LEFT_FRONT;
  stopDriveMotors();
  setMotorSpeed(MFRONT, -50);
}

void handleFrontLimitSwitchesAligned() {
  // sendSignal('2');
  sendSignal(ALIGNED);
  state = WAIT4DEST;

  // default testing instructions when serial comm isn't being received - uncomment when serial is working
  // state = MOVE2LEFT_1; 
  // stopDriveMotors();
  // moveRight(100);
}

void correctLimitSwitches() {
  setMotorSpeed(MRIGHT, digitalRead(PIN_LIMIT_RF) ? 0 : 50);
  setMotorSpeed(MLEFT, digitalRead(PIN_LIMIT_LF) ? 0 : 50);
}

void handleTape() {
  switch(state) {
    case MOVE2LEFT_1:
      state = SHOOT_LEFT_1;
      break;
    case MOVE2MID_1:
      state = SHOOT_MID_1;
      break;
    case MOVE2RIGHT:
      state = SHOOT_RIGHT;
      break;
    case MOVE2MID_2:
      state = SHOOT_MID_2;
      break;
    case MOVE2LEFT_2:
      state = SHOOT_LEFT_2;
  }
}

void handleReturnedLeft() {
  state = RETURN_BACK;
  stopDriveMotors();
  moveBack(100);
  TMRArd_InitTimer(TMR_RETURN, TMR_RETURN_VAL);
}

void handleReturnTimerExpired() {
  state = REFILL;
  TMRArd_ClearTimerExpired(TMR_RETURN);
  stopDriveMotors();
  TMRArd_InitTimer(TMR_REFILL, TMR_REFILL_VAL);
}

void handleRefillTimerExpired() {
  state = ALIGN_FRONT;
  TMRArd_ClearTimerExpired(TMR_REFILL);
  moveForward(100);
}

void handleNextGoal() { // Checks for a signal input from the flywheel controller of where to go next - sends output signal if none received  
  switch(inputSignal) {
    case NEXT_LEFT:
    state = MOVE2LEFT_1;
    break;

    case NEXT_MID:
    state = MOVE2MID_1;
    break;

    case NEXT_RIGHT:
    state = MOVE2RIGHT_1;
    break;

    case NEXT_REFILL:
    state = REFILL; // @Q: @TD: New state(s) needed for "return to refill" ?
    break;

    default:
    break;
  }
}

/*void updateSignal() {
  if(Serial.available()) {
    inputSignal = Serial.read();
  } else {
    inputSignal = NONE; // @Q: chars are single quotes - is that what I should use?
  }
  Serial.read(); // reading new line character
}

void sendSignal(char signal) {
  Serial.write(signal);
}*/

void setupPins() {
  pinMode(PIN_LIMIT_BL, INPUT);
  pinMode(PIN_LIMIT_FL, INPUT);
  pinMode(PIN_LIMIT_LF, INPUT);
  pinMode(PIN_LIMIT_RF, INPUT);
  pinMode(PIN_TAPE,     INPUT);

  pinMode(PIN_IR_ALIGN, INPUT);
}
