#ifndef SIMCALL_R1MF_H
#define SIMCALL_R1MF_H

// +----------+--------------------+-------------------------+
// | Keypad   | MF Signaling Tone  | Frequency Pair (Hz)     |
// +----------+--------------------+-------------------------+
// |    1     | Digit 1            |  700 Hz  +  900 Hz      |
// |    2     | Digit 2            |  700 Hz  + 1100 Hz      |
// |    3     | Digit 3            |  900 Hz  + 1100 Hz      |
// |    4     | Digit 4            |  700 Hz  + 1300 Hz      |
// |    5     | Digit 5            |  900 Hz  + 1300 Hz      |
// |    6     | Digit 6            | 1100 Hz  + 1300 Hz      |
// |    7     | Digit 7            |  700 Hz  + 1500 Hz      |
// |    8     | Digit 8            |  900 Hz  + 1500 Hz      |
// |    9     | Digit 9            | 1100 Hz  + 1500 Hz      |
// |    0     | Digit 0 (or 10)    | 1300 Hz  + 1500 Hz      |
// +----------+--------------------+-------------------------+
// |    *     | KP1 (Domestic)     | 1100 Hz  + 1700 Hz      |
// |  [Custom]| KP2 (International)| 1300 Hz  + 1700 Hz      |
// |    #     | ST (Start)         | 1500 Hz  + 1700 Hz      |
// +----------+--------------------+-------------------------+
// |    D     | SF Control Tone    | 2600 Hz  (Single Pure)  |
// +----------+--------------------+-------------------------+

#include <Arduino.h>

class R1mf
{
public:
  static int freqa_from_key(uint8_t key);

  static int freqb_from_key(uint8_t key);

private:
  static const int _freqa[16] PROGMEM;
  static const int _freqb[16] PROGMEM;
};

#endif
