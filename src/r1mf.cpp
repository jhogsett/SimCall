#include "r1mf.h"

// char keymap[20] = "D#0*C987B654A321INF"; // I - Invalid?, N = NoKey, F = Fail

// keymap specifies the characters associated with the 4x4 keypad from the bottom right key to the top left key (right->left, then down->up)
// this is the same order the I2CKeyPad expects

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

const int R1mf::_freqa[16] PROGMEM = {
  2600, // D - SF
  1500, // # - ST
  1300, // 0
  1100, // * - KP1
  1500, // C - ST
  1100, // 9
  900,  // 8
  700,  // 7
  1300, // B - KP2
  1100, // 6
  900,  // 5
  700,  // 4
  1100, // A - KP1
  900,  // 3
  700,  // 2
  700   // 1      
};

const int R1mf::_freqb[16] PROGMEM = {
  0,    // D - SF
  1700, // # - ST
  1500, // 0
  1700, // * - KP1
  1700, // C - ST
  1500, // 9
  1500, // 8
  1500, // 7
  1700, // B - KP2
  1300, // 6
  1300, // 5
  1300, // 4
  1700, // A - KP1
  1100, // 3
  1100, // 2
  900   // 1       
};

int R1mf::freqa_from_key(uint8_t key){
    key = key % 16;
    return pgm_read_word(&_freqa[key]);  
}

int R1mf::freqb_from_key(uint8_t key){
    key = key % 16;
    return pgm_read_word(&_freqb[key]);  
}