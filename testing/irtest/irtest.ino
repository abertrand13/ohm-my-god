#include <Timers.h>


#define YELLOW A0
#define RED A1
#define GREEN A2
#define SERIAL_DEBUG 0
#define RUN_FLY 1

// IR sensor 
#define PIN_IR_ALIGN A3 
#define PIN_IR_TOWER A4 



void setup() {
  Serial.begin(9600);
  TMRArd_InitTimer(0, 100);


  pinMode(PIN_IR_ALIGN, INPUT);
  pinMode(PIN_IR_TOWER, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
}	

void loop() {
	if(TMRArd_IsTimerExpired(0)) {
  		Serial.println(analogRead(PIN_IR_TOWER));
    	TMRArd_InitTimer(0, 100);
	}
}