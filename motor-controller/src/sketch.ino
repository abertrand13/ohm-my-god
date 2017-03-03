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
#include <Motors.h>

/*---------------Module Defines-----------------------------*/

// Pinout

// Limit Switches
#define PIN_LIMIT_BL A0     // Back limit switch on left side
#define PIN_LIMIT_FL A1     // Front limit switch on left side
#define PIN_LIMIT_LF A2     // Left limit switch on front side
#define PIN_LIMIT_RF A3     // Right limit switch on front side

// IR sensor 
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
bool checkLeftLimitSwitchesAligned(void);
bool checkFrontLimitSwitchesAligned(void);
bool checkTape(void);
void handleIRAlign(void);
void handleTurnTimerExpired(void);
void handleLeftLimitSwitchesAligned(void);
void handleFrontLimitSwitchesAligned(void);
void handleTape(void);
void handleReturnedLeft(void);
void handleReturnTimerExpired(void);
void handleRefillTimerExpired(void);
void setupSensorPins(void);

// Temporary Testing functions 
void printStates(void);

enum globalState {
  ALIGN_IR,       // Get initial bearings with IR
  ALIGN_TURN,     // Turn to face loosely right
  ALIGN_LEFT,     // Move to hug the left wall
  ALIGN_FRONT,    // Move to hug the front wall
  MOVE2LEFT_1,    // Move to face the left goal
  SHOOT_LEFT_1,   // Shoot on the left goal
  MOVE2MID_1,     // Move to face the middle goal
  SHOOT_MID_1,    // Shoot on the middle goal
  MOVE2RIGHT,     // Move to face the right goal
  SHOOT_RIGHT,    // Shoot on the right goal
  MOVE2MID_2,     // Move to face the middle goal
  SHOOT_MID_2,    // Shoot on the middle goal
  MOVE2LEFT_2,    // Move to face the left goal
  SHOOT_LEFT_2,   // Shoot on the left goal
  RETURN_LEFT,    // Move left to return to safe space
  RETURN_BACK,    // Move back to return to safe space
  REFILL          // Pause in the safe space for refill
};

/*---------------Module Variables---------------------------*/
enum globalState state;
bool onTape;

/*---------------Main Functions-----------------------------*/
void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();

  // setMotorSpeed(MLEFT, 100);
  // setMotorSpeed(MRIGHT, 100);
  TMRArd_InitTimer(TIMER_0, ONE_SEC);
  state = ALIGN_TURN;
}

void loop() { 
  // control routine (temporary)
  // to operate, just enter commands in the serial port
  // eg:
  // l127 sets left motor to full forward speed
  // l-127 sets left motor to full reverse speed
  // f127 set front motor to full forward speed
  // b-128 reverses the back motor, whether it's going forward or backward
  if(Serial.available()) {
    char motor = Serial.read();
    enum motorID select;
    if(motor == 'l') {
      select = MLEFT;
    } else if(motor == 'r'){
      select = MRIGHT;
    } else if(motor == 'b'){
      select = MBACK;
    } else if(motor == 'f'){
      select = MFRONT;
    }
    int x = Serial.parseInt();
    Serial.println(x);

    setMotorSpeed(select, char(x));

    // if(x != -128) {
    //   setMotorSpeed(select, char(x));
    // } else {
    //   flipMotorDirection(select);
    // }
    Serial.read(); //newline
  }
  
  checkEvents();

  applyMotorSettings();

  if(TMRArd_IsTimerExpired(TIMER_0)) printStates();    
}

void printStates(void) {
  if(checkFrontLimitSwitchesAligned()) Serial.println("FRONT SWITCHES ALIGNED");
  if(checkLeftLimitSwitchesAligned()) Serial.println("LEFT SWITCHES ALIGNED");

  Serial.print("state = ");

  switch(state) {
      case ALIGN_IR:
      Serial.println("ALIGN_IR");
      break;
      
      case ALIGN_TURN:
      Serial.println("ALIGN_TURN");
      break;

      case ALIGN_LEFT:
      Serial.println("ALIGN_LEFT");
      break;

      case ALIGN_FRONT:
      Serial.println("ALIGN_FRONT");
      break;

      case MOVE2LEFT_1:
      Serial.println("MOVE2LEFT_1"); 
      break;

      case MOVE2MID_1:
      Serial.println("MOVE2MID_1"); 
      break; 

      case MOVE2RIGHT:
      Serial.println("MOVE2RIGHT");
      break;

      case MOVE2MID_2:
      Serial.println("MOVE2MID_2");

      case MOVE2LEFT_2:
      break;

      Serial.println("MOVE2LEFT_2");
      break;

      case RETURN_LEFT:
      Serial.println("RETURN_LEFT");
      break;

      case RETURN_BACK:
      Serial.println("RETURN_BACK");
      break;

      case REFILL:
      Serial.println("REFILL");
      break;
    }

  Serial.println("~~~~~~~~~~~~~");


  TMRArd_InitTimer(TIMER_0, ONE_SEC);
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
  switch(state) {
    case ALIGN_IR:
      if (checkIRAlign()) { handleIRAlign(); }
      break;
    
    case ALIGN_TURN:
      if (TMRArd_IsTimerExpired(TMR_ALIGN)) { handleTurnTimerExpired(); }
      break;

    case ALIGN_LEFT:
      if(checkLeftLimitSwitchesAligned()) handleLeftLimitSwitchesAligned();
      break;

    case ALIGN_FRONT:
      if(checkFrontLimitSwitchesAligned()) { handleFrontLimitSwitchesAligned(); }
      break;

    case MOVE2LEFT_1: 
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
  return digitalRead(PIN_IR_ALIGN);
}

/******************************************************************************
  Function:    checkLeftLimitSwitchesAlign
  Contents:    Checks to see whether or not we have made contact with both
               limit switches on the left side of the robot. The assumption
               is that once we have, we are flush with the left wall of
               the echo chamber.
  Parameters:  None
  Returns:     Whether both limit switches have been pressed, {true, false}.
  Notes:       Assumes that limit switches have been wired in the
               'Normally Open' state.
******************************************************************************/

bool checkLeftLimitSwitchesAligned() {
  return analogRead(PIN_LIMIT_BL) & analogRead(PIN_LIMIT_FL);
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
  state = ALIGN_TURN;
  TMRArd_InitTimer(TMR_ALIGN, TMR_ALIGN_VAL); 
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

void handleFrontLimitSwitchesAligned() {
  state = MOVE2LEFT_1;
  stopDriveMotors();
  moveRight(100);
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

void setupPins() {
  pinMode(PIN_LIMIT_BL, INPUT);
  pinMode(PIN_LIMIT_FL, INPUT);
  pinMode(PIN_LIMIT_LF, INPUT);
  pinMode(PIN_LIMIT_RF, INPUT);
  pinMode(PIN_TAPE,     INPUT);

  pinMode(PIN_IR_ALIGN, INPUT);
}
