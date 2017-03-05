# OHM-MY-GOD

Courtesy, the Bowling Green Massacre.

Eni Asebiomo // Cuthbert Sun // Chris Kimes // Alex Bertrand

## Signaling Protocol
Signals are enumerated as 1 of a possible list, and sent serially from arduino to arduino.

### Possible signals
- "1" = The IR beacon in the safe space was detected. This doesn't need to be precise (at all) - we just need to make sure that when we send this signal we're facing loosely forward. Signal sent from the Flywheel Controller to the Motor Controller.

- "2" = Initial alignment has been reached. Here, we interpret initial alignment as being back into the front left corner of the field. Signal sent from the Motor Controller to the Flywheel Controller, right after we back into the corner.

- "3", "4", "5", "6" = The 'Next Goal' signals (chosen from Left, Center, Right, Refill). This signal tells the motor controller where to move to next. Note that internal state may have to be kept synchronized between the Flywheel Controller and the Motor Controller. Fortunately, this shouldn't be too difficult.

- "7" = Ready to Fire. This signal is sent from the Motor Controller to the Flywheel Controller and indicates that we've aligned the bot with the goal that was previously requested with the 'Next Goal' signal. When the Flywheel Controller is done firing balls, it should respond with a Next Goal signal to tell the Motor Controller where to go next.

- "8" = Done Refilling. Sent from the Motor Controller to the Flywheel Controller. The Flywheel Controller should respond with a Next Goal signal to tell the Motor Controller where to go. Note that this is technically redundant and could probably be implemented using only the Ready to Fire signal, but since we're not particularly constrained on space here we're separating things out for clarity.

### Sample Signalling Sequence
1. "1" - IR Beacon located
2. "2" - Initial alignment reached
3. "3" - Flywheel Controller tells the Motor Controller to go to the Left Goal
4. "7" - Motor Controller moves to the Left Goal. When it has reached it, it sends this signal to let the Flywheel Controller to fire away.
5. "4" - Based on internal strategy implementation, the Flywheel Controller fires a certain number of balls at the Left Goal and determines that we should then move to the Center Goal.
6. "7" - The Motor Controller moves to the Center Goal and tells the Flywheel Controller to fire again.
7. "6" - The Flywheel Controller finishes firing and determines we need to refill.
8. "8" - Sent by the motor controller after we are done refilling.
9. "3" - The Flywheel Controller tells the Motor Controller to move to the Left Goal.
10. "7" - Ready to fire.
11. etc.

### Internal Implementation

I'm envisioning a pretty simple scheme here:
1. At the beginning of checkEvents(), each FSM will read in a serial character if one is available. That will go into some variable `signal`. If there's no signal available then `signal` will be set to null.
2. Any states that rely on signals from the arduino will run a 'check signal' sort of function. That can be done inline (in the `case` statement), or we can just call a function and pass in `signal`. Note that we're going to have to create some new states. One example that immediately comes to mind is a state for 'wait for destination' that checks to see if a received signal was a 3, 4, 5, or 6, and then if it was updates the state to something like 'MOVE2LEFT' and begins moving. If it wasn't, it should do nothing. It won't block, but it will keep us motionless (which is what we want).

That method - of reading something in every cycle, but only using it if we're in the proper state - should make it a lot simpler for us to coordinate because we won't have to worry about distinguishing whether a signal is relevant or not and weeding it out from any other noise.
