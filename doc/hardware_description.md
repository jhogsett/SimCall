## SimCall Device

### Background

I like building simulators. In 2001 I build a telephone call simulator based on the Parallax Basic Stamp-2. That processor had a built-in method for generatoring two simultaneous (sort-of) sinewaves, perfect for producing almost all telephone signaling tones. The device made automatic random simulated telephone calls, including local, long distance and overseas to the UK. 

This device is similar, but instead of random calls, it has a 4x4 DTMF keypad (including the fourth column 'A' through 'D' keys.) It uses an Arduino Nano processor and two AD9833 modules to produce the sounds. The output from the AD9833 modules are feed into an amplifier and speaker.

### Hardware Description

* Arduino Nano (clone)
* 4x4 DTMF Keypad with I2C interface
* (2) AD9833 Devices with SPI interface
* Off-Hook Pilot Light LED
* PAM8403 amplifier
* Speaker
* Li-Ion 18500 battery
* Lithum battery charging device with Micro USB charging input
* Power Switch
* 3D printed enclosure

### Software Description

#### Current Development

* Press 'A' key and get a dialtone
    * Press 'A' again to cancel and hear a "cancellation" tone
* With dialtone present, begin dialing
    * Dial a local 7-digit number
    * Dial starting with '1' to dial a long-distance number
    * Dial starting with '011' to dial an international number
    * Press 'A' at any time while dialing to cancel the call
* Once the correct number of digits are dialed, a call routing is simulated resulting on one of four outcomes
    * Ringback Tone (call placed normally)
    * Busy Tone
    * Reorder Tone (all circuits busy)
    * Error tone (number is invalid)
* When dialing an international number, the telephone signals for the United Kingdom are played
* The routing is chosen based on a probability distribution
    * Local calls
        * 73% chance of ringback
        * 25% chance of busy 
        * 0.5% chance of reorder
        * 1.5% cchance of error
    * Long-Distance calls
        * 68% chance of ringback
        * 23% chance of busy
        * 6% chance of reorder
        * 3% chance of error
    International Calls
        * 58% chance of ringback
        * 25% chance of busy
        * 12% chance of reorder
        * 5% change of error

#### Future Development

* Pressing 'B' will enter "operator" or "blue box" mode
    * Instead of a dial-tone, the 2600 Hz disconnect (or Captain Crunch) tone will be heard for one second, simulating grabbing an open trunk
    * The numeric keys will use the alternate set of DTMF tones used by operators or phone phreaks
    * '*' and '#' will be mapped to KP1 and SP to start and end the sequence of digits
* Pressing 'C' will enter the International version of the "operataor" mode
    * '*' will be presseed to start a digit sequence
    * Individual digits will be cached and generate a simple tone
    * When '#' is pressed, the digits will be sequenced automatically starting with KP1 and ending with KP2, simulating the auto-dialed digit sequence heard in Pink Floyd The Wall at the end of the song "Young Lust"
* Pressing 'D' will enter "Free" mode where each key will be mapped to a different telephone sound, played while pressed, for example
    * dial-tone, Ringback, busy tone, reorder tone, error tones
    * UK versions of those
    * Pay telephone coin inserted signal tones
    * 2600 Hz disconnect tone
    * Others
