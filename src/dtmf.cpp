#include "dtmf.h"

// char keymap[20] = "D#0*C987B654A321INF"; // I - Invalid?, N = NoKey, F = Fail

// keymap specifies the characters associated with the 4x4 keypad from the bottom right key to the top left key (right->left, then down->up)
// this is the same order the I2CKeyPad expects

// Dtmf::Dtmf(){
//   _rows[0] = 697;
//   _rows[1] = 770;
//   _rows[2] = 852;
//   _rows[3] = 941;
//   _cols[0] = 1209;
//   _cols[1] = 1336;
//   _cols[2] = 1477;
//   _cols[3] = 1633;
// }

constexpr int Dtmf::_rows[4];
constexpr int Dtmf::_cols[4];

int Dtmf::row_freq_from_key(uint8_t key){
    key = key % 16;
    uint8_t row = (15 - key) / 4;
    return _rows[row];
}

int Dtmf::col_freq_from_key(uint8_t key){
    key = key % 16;
    uint8_t col = (15 - key) % 4;
    return _cols[col];
}
