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

// IR sensor 
#define PIN_IR_ALIGN A4     // IR sensor to align in safe space @TD: Update this to actual value

// Constants
#define BALL_CAPACITY 9

// Firing
#define TMR_FIRE 7			// Timer to control firing feed
#define FIRE_CONSTANT 500	// Time to feed per ball

enum globalState {
	ALIGN_IR, 			// Getting initial bearings with IR
	WAIT4ALIGN, 		// IR beacon has been located, and bot is finding wallt
	FIND_DEST,			// Logic to find next destination
	MOVE2DEST,			// Moving to next destination
	FIRING,				// IMMA FIRIN MAH LAZERRRR
	REFILLING			// Moving to safe space and refilling
};

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);
void checkEvents(void);

// Logic Functions
bool checkIRAlign(void);
void handleIRAlign(void);

void findAndSendDestination(void);
void handleReadyToFire(void);
void fireAway(void);
void handleDoneRefilling(void);

/* Signal Functions
// now pulled out into separate library
void updateSignal(void);
void sendSignal(char signal);*/


/*---------------Module Variables---------------------------*/

enum globalState state;
enum signal inputSignal;
int ballsLeft;
int ballsToFire;
enum location location;
enum location destination;

/*---------------Main Functions-----------------------------*/

void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();

  state = ALIGN_IR;

  setFlywheelMotorSpeed(50);

  // Timer for testing serial comms
  TMRArd_InitTimer(0, 1000);
}

void loop() {
    /*if(Serial.available()) {
      char motor = Serial.read();
      int x = Serial.parseInt();
      Serial.println(x);

      setFlywheelMotorSpeed(char(x));

      // if(x != -128) {
      //   setMotorSpeed(select, char(x));
      // } else {
      //   flipMotorDirection(select);
      // }
       Serial.read(); //newline @Q: Why is this here?


  }*/

  checkEvents();

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
  inputSignal = receiveSignal();

  switch(state) {
  	case ALIGN_IR:
  	  // if (checkIRAlign()) { handleIRAlign(); } // Actual code 
  	  if(TMRArd_IsTimerExpired(0)) handleIRAlign(); // Testing Code for serial comm
      break;
  	
	case WAIT4ALIGN:
  	  if(inputSignal == ALIGNED) { state = FIND_DEST; }
	  break;
	
	case FIND_DEST: // @TD this should be broken into two states (decide instructions and send? Idk) I think we're actually good. I know what you mean though, I was thinking the same thing.
	  findAndSendDestination();
	  break;
    
	case MOVE2DEST:
	  if(inputSignal == READY2FIRE) { handleReadyToFire(); }
      break;

	case FIRING:
	  fireAway();
	  break;

	case REFILLING:
	  if(inputSignal == REFILL_DONE) { handleDoneRefilling(); }
	  break;
  }
}

bool checkIRAlign() {
  return digitalRead(PIN_IR_ALIGN); // @TD: make this the right pin
}

void handleIRAlign() {
  // pinMode(13, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	// delay(1000);
	// digitalWrite(13, LOW);
	state = WAIT4ALIGN;
  	sendSignal(FOUND_IR);
	delay(500);
	digitalWrite(LED_BUILTIN, LOW);
}

void findAndSendDestination() {
  /* NOTE:
	 THIS RIGHT HERE IS THE BIG ENCHILADA
	 ALL OUR STRATEGY GOES HERE
	 SHIT GUYZ
	 */

  if(ballsLeft <= 0) {
	// if we have no balls left we should refill
	destination = REFILL;
	sendSignal(NEXT_REFILL);
	state = REFILLING;
  } else {
    // simple, move right and then refill
	switch(location) {
	  case REFILL:
	    destination = GOAL_LEFT;
		sendSignal(NEXT_LEFT);
		break;

      case GOAL_LEFT:
        destination = GOAL_MID;
		sendSignal(NEXT_MID);
		break;
      
	  case GOAL_MID:
		destination = GOAL_RIGHT;
		sendSignal(NEXT_RIGHT);
		break;
	
	  case GOAL_RIGHT:
		destination = REFILL;
		sendSignal(NEXT_REFILL);
        break;
    }
  }

  state = MOVE2DEST;
}

void handleReadyToFire() {
  location = destination;
  state = FIRING;
}

void fireAway() {
  switch(TMRArd_IsTimerExpired(TMR_FIRE)) {
    case TMRArd_EXPIRED:
	  // Timer has run out (we're done firing)
	  state = FIND_DEST;
	  TMRArd_ClearTimerExpired(TMR_FIRE);
	  digitalWrite(LED_BUILTIN, LOW);
	  break;
	
	case TMRArd_ERR:
	  ballsToFire = 3;
	  // this will have to be more involved later, but...
	  // start motors
	  digitalWrite(LED_BUILTIN, HIGH);
	  TMRArd_InitTimer(TMR_FIRE, ballsToFire * FIRE_CONSTANT);
	  break;

	case TMRArd_NOT_EXPIRED:
	  // sit tight while balls fire
	  break;
  }	
}

void handleDoneRefilling() {
  if(inputSignal == REFILL_DONE) {
   	ballsLeft = BALL_CAPACITY;
    state = WAIT4ALIGN;
  }
}

/*void updateSignal() {
  if(Serial.available()) {
		inputSignal = Serial.read();
	} else {
		inputSignal = '0'; // @Q: chars are single quotes - is that what I should use? Yes.
	}
	Serial.read(); 
}

void sendSignal(char signal) {
	Serial.write(signal);
}*/

void setupPins() {
  pinMode(PIN_IR_ALIGN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

