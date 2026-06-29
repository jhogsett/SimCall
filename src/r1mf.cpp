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

R1mf::R1mf(){
  _freqa[0] = 2600; // D - SF
  _freqa[1] = 1500; // # - ST
  _freqa[2] = 1300; // 0
  _freqa[3] = 1100; // * - KP1
  _freqa[4] = 1500; // C - ST
  _freqa[5] = 1100; // 9
  _freqa[6] = 900; // 8
  _freqa[7] = 700; // 7
  _freqa[8] = 1100; // B - KP1
  _freqa[9] = 1100; // 6
  _freqa[10] = 900; // 5
  _freqa[11] = 700; // 4
  _freqa[12] = 1100; // A - KP1
  _freqa[13] = 900; // 3
  _freqa[14] = 700; // 2
  _freqa[15] = 700; // 1

  _freqb[0] = 0; // D - SF
  _freqb[1] = 1700; // # - ST
  _freqb[2] = 1500; // 0
  _freqb[3] = 1700; // * - KP1
  _freqb[4] = 1700; // C - ST
  _freqb[5] = 1500; // 9
  _freqb[6] = 1500; // 8
  _freqb[7] = 1500; // 7
  _freqb[8] = 1700; // B - KP2
  _freqb[9] = 1300; // 6
  _freqb[10] = 1300; // 5
  _freqb[11] = 1300; // 4
  _freqb[12] = 1700; // A - KP1
  _freqb[13] = 1100; // 3
  _freqb[14] = 1100; // 2
  _freqb[15] = 900; // 1

}

int R1mf::freqa_from_key(uint8_t key){
    key = key % 16;
    return _freqa[key];  
}

int R1mf::freqb_from_key(uint8_t key){
    key = key % 16;
    return _freqb[key];  
}