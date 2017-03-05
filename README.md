# OHM-MY-GOD

Courtesy, the Bowling Green Massacre.

Eni Asebiomo // Cuthbert Sun // Chris Kimes // Alex Bertrand

## Signaling Protocol
Signals are enumerated as 1 of a possible list, and sent serially from arduino to arduino.

### Possible signals
- "1" = The IR beacon in the safe space was detected. This doesn't need to be precise (at all) - we just need to make sure that when we send this signal we're facing loosely forward. Signal sent from the Flywheel Controller to the Motor Controller.
