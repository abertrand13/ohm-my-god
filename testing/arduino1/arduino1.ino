#include <Signals.h>
#include <Timers.h>

enum signal inputSignal;

void setup() {
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	delay(100);
	digitalWrite(13, LOW);
	TMRArd_InitTimer(0, 1000);
}

void loop() {
	if(TMRArd_IsTimerExpired(0)) {
		digitalWrite(13, HIGH);
		delay(100);
		digitalWrite(13, LOW);
		delay(100);
		digitalWrite(13, HIGH);
		delay(100);
		digitalWrite(13, LOW);

		
		signal sig = NEXT_MID;
		sendSignal(sig);
		// Serial.write(2);

		TMRArd_ClearTimerExpired(0);
		TMRArd_InitTimer(0, 500);
	}
}
