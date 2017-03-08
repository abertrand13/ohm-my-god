#include <Timers.h>

void setup() {
	pinMode(A4, INPUT);
	Serial.begin(9600);
    TMRArd_InitTimer(0, 100);
}

void loop() {
	// Serial.print('A4 = ');
	if(TMRArd_IsTimerExpired(0)) {
		Serial.println(analogRead(A4));
    	TMRArd_InitTimer(0, 100);
	}
}
