#include <Signals.h>
#include <Timers.h>

enum signal inputSignal;

void setup() {
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	delay(100);
	digitalWrite(13, LOW);
	TMRArd_InitTimer(0, 3000);
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

		
		signal sig = ALIGNED;
		sendSignal(sig);
	}
}