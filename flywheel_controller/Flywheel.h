/*=============================================================================
 
  Flywheel.h

=============================================================================*/

#include "Arduino.h"

/*---------------Module Defines-----------------------------*/

// Pinout
#define PIN_FLY_EN 6    	// Output pin for H Bridge
#define PIN_FLY_A 5			// One side of drive
#define PIN_FLY_B 7			// Other side of drive

#define PINS_PER_MOTOR 3

/*---------------------------Public Functions--------------------------------*/

void applyMotorSettings(void);
char getFlywheelMotorSpeed(void);
void setFlywheelMotorSpeed(char val);
void stopFlywheelMotor(void);
void setupMotorPins(void);

// Fast PWM
void SetupTimerInterrupt(void);
void updatePWM(void);