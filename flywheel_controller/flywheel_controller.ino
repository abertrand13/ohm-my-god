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


/*---------------Module Defines-----------------------------*/

// Pinout

// IR sensor 
#define PIN_IR_ALIGN A4     // IR sensor to align in safe space @TD: Update this to actual value

/*---------------Module Function Prototypes-----------------*/
void setupPins(void);
void checkEvents(void);

// Logic Functions
bool checkIRAlign(void);
void handleIRAlign(void);

// Signal Functions
void updateSignal(void);
void sendSignal(char signal);

enum globalState {
	ALIGN_IR, // Getting initial bearings with IR
	IR_FOUND, // IR beacon has been located, and commm is being sent
	SEND_INSTRUCTIONS, // @TD: change this name to something better 
	// - is a placeholder to say "I know you've received my instructions, 
  WAIT
};

/*---------------Module Variables---------------------------*/
enum globalState state;
char inputSignal;

/*---------------Main Functions-----------------------------*/

void setup() {
  Serial.begin(9600);
  setupPins();
  setupMotorPins();


  state = ALIGN_IR;

  // SetupTimerInterrupt();
  // InitPulse(PIN_FLY_EN, 990);
  // Pulse(10);
  // Timer for testing serial comms
  // TMRArd_InitTimer(0, 5000);
}

void loop() {
    if(Serial.available()) {
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


  	// checkEvents();
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
  updateSignal();

  switch(state) {
  	case ALIGN_IR:
  		// if (checkIRAlign()) { handleIRAlign(); } // Actual code 
  		if(TMRArd_IsTimerExpired(0)) handleIRAlign(); // Testing Code for serial comm
    	break;
  	case IR_FOUND:
  		if(inputSignal == '2') state = SEND_INSTRUCTIONS;
		break;
	  case SEND_INSTRUCTIONS: // @TD this should be broken into two states (decide instructions and send? Idk)
		  handleSendInstructions();
		break;
    case WAIT:
    break;
  }
}

bool checkIRAlign() {
  return digitalRead(PIN_IR_ALIGN); // @TD: make this the right pin
}

void handleIRAlign() {
  // pinMode(13, OUTPUT);
	// digitalWrite(13, HIGH);
	// delay(1000);
	// digitalWrite(13, LOW);
	state = IR_FOUND;
  sendSignal('1');
}

void handleSendInstructions() {
	// @TD: logic and stuff to decide what instructions to send
	sendSignal('3');
  state = WAIT;
}

void updateSignal() {
  if(Serial.available()) {
		inputSignal = Serial.read();
	} else {
		inputSignal = '0'; // @Q: chars are single quotes - is that what I should use?
	}
	Serial.read(); 
}

void sendSignal(char signal) {
	Serial.write(signal);
}

void setupPins() {
  pinMode(PIN_IR_ALIGN, INPUT);
}

