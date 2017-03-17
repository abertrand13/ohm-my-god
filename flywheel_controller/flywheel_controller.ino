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

#include "Flywheel.h"
#include <Timers.h>
#include <Signals.h>


/*---------------Module Defines-----------------------------*/

// Pinout

// Debugging Pins
#define YELLOW A0
#define RED A1
#define GREEN A2
#define SERIAL_DEBUG 0
#define RUN_FLY 1

// IR sensor 
#define PIN_IR_ALIGN A3 
#define PIN_IR_TOWER A4 

// Constants
#define BALL_CAPACITY 12
#define IR_ON_LOW 50
#define IR_50_HIGH 600 // Losing or Tied
#define IR_50_LOW 350
#define IR_25_HIGH 300 // Winning
#define IR_25_LOW 100

// Firing
#define FLYWHEEL_SPEED 56
#define TMR_GAS 8
#define TMR_GAS_VAL 2500
#define TMR_HOLD 9
#define TMR_HOLD_VAL 500

// Stopping
#define TMR_GAME 10
#define GAME_LENGTH 130000

enum globalState {
	ALIGN_IR, 			// Getting initial bearings with IR
	WAIT4ALIGN, 		// IR beacon has been located, and bot is finding wall
	FIND_DEST,			// Logic to find next destination
	MOVE2DEST,			// Moving to next destination
  HOLD_PRE_FIRE,	// Waiting for motors to stop
  FIRING,				  // IMMA FIRIN MAH LAZERRRR
  HOLD_POST_FIRE, // Waiting.
	REFILLING,			// Moving to safe space and refilling
  STOPPED
};

enum goalState {
  BLOCKED,
  LOSING,
  WINNING
};

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);
void checkEvents(void);

// Logic Functions
bool checkIRAlign(void);
goalState checkTowerAvailable(void);
void handleIRAlign(void);

void findAndSendDestination(void);
void handleReadyToFire(void);
void startHolding(void);
void fireAway(void);
void finishHolding(void);
void handleDoneRefilling(void);

/*---------------Module Variables---------------------------*/

enum globalState state;
enum signal inputSignal;
int ballsLeft;
int ballsToFire;
enum Location location;
enum Location destination;

/*---------------Main Functions-----------------------------*/

void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();
 
  state = ALIGN_IR;

  if(RUN_FLY) setFlywheelMotorSpeed(100);
  TMRArd_InitTimer(TMR_GAS, TMR_GAS_VAL);
  
  ballsLeft = BALL_CAPACITY;
  location = REFILL;

  // start game timer
  TMRArd_InitTimer(TMR_GAME, GAME_LENGTH);
}

void loop() {
  checkEvents();
  
  if(TMRArd_IsTimerExpired(TMR_GAS) == TMRArd_EXPIRED) {
    TMRArd_ClearTimerExpired(TMR_GAS);
	  if(RUN_FLY) setFlywheelMotorSpeed(FLYWHEEL_SPEED);
  }
  
  applyMotorSettings();
}

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

  if(TMRArd_IsTimerExpired(TMR_GAME)) {
    state = STOPPED;
    sendSignal(STOPAF);
    stopFlywheelMotor();
  }

  switch(state) {
  	case ALIGN_IR:
  	  if (checkIRAlign()) { handleIRAlign(); }
      break;
  	
	case WAIT4ALIGN:
  	  if(inputSignal == ALIGNED) { state = FIND_DEST; }
	  break;
	
	case FIND_DEST:
	  findAndSendDestination();
	  break;
    
	case MOVE2DEST:
	  if(inputSignal == READY2FIRE) { startHolding();}
    break;

  case HOLD_PRE_FIRE:
    handleReadyToFire();
    break;
  
  case FIRING:
	  fireAway();
	  break;

  case HOLD_POST_FIRE:
    finishHolding();
    break;

	case REFILLING:
	  if(inputSignal == REFILL_DONE) { handleDoneRefilling(); }
	  break;
  }
}

bool checkIRAlign() {
  return analogRead(PIN_IR_ALIGN) > IR_ON_LOW;
}

void handleIRAlign() {
	state = WAIT4ALIGN;
  sendSignal(FOUND_IR);	
}

void findAndSendDestination() {

  if(ballsLeft <= 0) {
	  // if we have no balls left we should refill
	  destination = REFILL;
	  sendSignal(NEXT_REFILL);
	  state = REFILLING;
  } else {
    // simple, move right and then refill
    switch(location) {
      case REFILL:
        digitalWrite(GREEN, !digitalRead(GREEN)); 
        destination = GOAL_LEFT;
        sendSignal(NEXT_LEFT);
        state = MOVE2DEST;
        break;

      case GOAL_LEFT:
        destination = GOAL_MID;
        sendSignal(NEXT_MID);
        state = MOVE2DEST;
        break;
        
      case GOAL_MID:
        destination = GOAL_RIGHT;
        sendSignal(NEXT_RIGHT);
        state = MOVE2DEST;
        break;
    
      case GOAL_RIGHT:
        state = REFILLING; 
        destination = REFILL;
        sendSignal(NEXT_REFILL);
        break;	  
    }
  }
}

void startHolding() {
  location = destination;
  state = HOLD_PRE_FIRE;
  TMRArd_InitTimer(TMR_HOLD, TMR_HOLD_VAL);
}

void handleReadyToFire() {
  if(TMRArd_IsTimerExpired(TMR_HOLD)) {
    TMRArd_ClearTimerExpired(TMR_HOLD);
    enum goalState stateOfGoal = checkTowerAvailable(); 
    
    switch(stateOfGoal) {
      case BLOCKED:
        switch(location) {
          case GOAL_LEFT:
            ballsToFire = 2;
            break;
          case GOAL_RIGHT:
            ballsToFire = ballsLeft;
            break;
          default:
            ballsToFire = 0;
            break;
        }
        break;

      case LOSING:
        switch(location) {
          case GOAL_LEFT:
            ballsToFire = 8;
            break;

          case GOAL_MID:
            ballsToFire = ballsLeft;
            break;

          case GOAL_RIGHT:
            ballsToFire = ballsLeft;
            break;
        }
        break;

      case WINNING:
        switch(location) {
          case GOAL_RIGHT:
            ballsToFire = ballsLeft;
            break;

          default:
            ballsToFire = 2;
            break;
        }
        break; 
    }
     
    feedBalls(ballsToFire);
    state = FIRING;
  } 
}

void fireAway() {
  if(doneFeeding()) {
    ballsLeft -= ballsToFire;
    TMRArd_InitTimer(TMR_HOLD, TMR_HOLD_VAL);
    state = HOLD_POST_FIRE;
  }
}

void finishHolding() {
  if(TMRArd_IsTimerExpired(TMR_HOLD) == TMRArd_EXPIRED) {
    TMRArd_ClearTimerExpired(TMR_HOLD);
    state = FIND_DEST;
  }
}

void handleDoneRefilling() {
  digitalWrite(YELLOW, HIGH); 
  ballsLeft = BALL_CAPACITY;
  state = WAIT4ALIGN;
  location = REFILL;
}

goalState checkTowerAvailable(void) {
  int val = analogRead(PIN_IR_TOWER);
  if(val <= IR_ON_LOW) {
    return BLOCKED;
  } else if(val >= IR_25_LOW && val <= IR_25_HIGH) {
    return WINNING;
  } else if(val >= IR_50_LOW && val <= IR_50_HIGH) {
    return LOSING;
  } else {
    return WINNING; // Because when in doubt, we win.
  }
}

void setupPins() {
  pinMode(PIN_IR_ALIGN, INPUT);
  pinMode(PIN_IR_TOWER, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

