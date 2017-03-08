/*=============================================================================
 
  Flywheel.h

=============================================================================*/

#include "Arduino.h"

/*---------------Module Defines-----------------------------*/

// Pinout
#define PIN_FLY_EN 6    	// Output pin for H Bridge
#define PIN_FLY_A 5			// One side of drive
#define PIN_FLY_B 7			// Other side of drive

#define PIN_FEED_EN 3		// Feed motor controls
#define PIN_FEED_A 4
#define PIN_FEED_B 2

#define PINS_PER_MOTOR 3

/*---------------------------Public Functions--------------------------------*/

void applyMotorSettings(void);
char getFlywheelMotorSpeed(void);
void setFlywheelMotorSpeed(char val);
void stopFlywheelMotor(void);
void setFeedMotorSpeed(char val);
void stopFeedMotor(void);
void setupMotorPins(void);

// Slow PWM
// It's all about the slow pwm :D
void updateDutyCycle(void);
