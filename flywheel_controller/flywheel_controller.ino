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
#define RUN_FLY 0

// IR sensor 
#define PIN_IR_ALIGN A3 
#define PIN_IR_TOWER A4 

// Constants
#define BALL_CAPACITY 12
#define IR_ON_LOW 250
#define IR_50_HIGH 600 // Losing or Tied
#define IR_50_LOW 400
#define IR_25_HIGH 300 // Winning
#define IR_25_LOW 150

#define TOWER_IR_ON_LOW 150 // Lower threshold to detect any IR at all

// Firing
// #define TMR_FIRE 7			// Timer to control firing feed
// #define FIRE_CONSTANT 250	// Time to feed per ball
#define FLYWHEEL_SPEED 52
#define TMR_GAS 8
#define TMR_GAS_VAL 2000
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

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);
void checkEvents(void);

// Logic Functions
bool checkIRAlign(void);
bool checkTowerAvailable(void);
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
 
  // state = ALIGN_IR;
  state = WAIT4ALIGN;

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
  	  if (checkIRAlign()) { handleIRAlign(); } // Actual code 
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
    if(checkTowerAvailable()) {
      ballsToFire = 4;
    } else {
      ballsToFire = 0;
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

bool checkTowerAvailable(void) {
  return analogRead(PIN_IR_TOWER) > TOWER_IR_ON_LOW;
}

void setupPins() {
  pinMode(PIN_IR_ALIGN, INPUT);
  pinMode(PIN_IR_TOWER, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

