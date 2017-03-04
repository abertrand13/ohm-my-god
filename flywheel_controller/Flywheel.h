/*=============================================================================
 
  Flywheel.h

=============================================================================*/

#include "Arduino.h"

/*---------------Module Defines-----------------------------*/

// Pinout
#define PIN_FLY_EN 3    	// Output pin for H Bridge
#define PIN_FLY_A 2			// One side of drive
#define PIN_FLY_B 4			// Other side of drive

#define PINS_PER_MOTOR 3

/*---------------------------Public Functions--------------------------------*/

void applyMotorSettings(void);
bool getFlywheelMotorForward(void);
char getFlywheelMotorSpeed(void);
void setFlywheelMotorSpeed(char val);
void stopFlywheelMotor(void);
void flipFlywheelDirection(void);
void setupMotorPins(void);
