## June 28 2026

- [ ] add a timer reset to the non-blocking timer
- [ ] have a top-level time-out if the device is left on waiting for 'A' through 'D'
- [ ] have a dialing-in-progress time-out to play an error if the dialing wasn't finished, or to handle variable dialing digits
- [ ] handle calls to 911, 0, etc.
- [ ] look into making more pronounced pop and click sounds
- [ ] play the R1MF tones on international calls
- [ ] implement a mid-twentieth century _howler_ sound given it can be produced with just two sine waves
    - one tone sweeps from 800 Hz to 3200 Hz over the course of 12 seconds, then back down in reverse
    - a second tone does the same but 1 Hz lower to create a 1 Hz warble sound
- [ ] have special secret numbers like 867-5309
- [ ] have a continuous automatic dialing mode
- [ ] have a semi-automatic dialing mode (press 'C' to dial a new random number)
- [ ] have a "redial" feature (press 'D' to redial the last dialed number)
- [ ] 'B' through 'D' hang up while dialing


need char to key conversion using keymap
make dtmf and r1mf static
fix use of "B" in op mode (shouldn't be kp1)
pressing numeric key ends call while in routing state
