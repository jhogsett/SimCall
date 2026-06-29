#include "dtmf.h"

// char keymap[20] = "D#0*C987B654A321INF"; // I - Invalid?, N = NoKey, F = Fail

// keymap specifies the characters associated with the 4x4 keypad from the bottom right key to the top left key (right->left, then down->up)
// this is the same order the I2CKeyPad expects

const int Dtmf::_rows[4] PROGMEM = { 697, 770, 852, 941 };
const int Dtmf::_cols[4] PROGMEM = { 1209, 1336, 1477, 1633 };

int Dtmf::row_freq_from_key(uint8_t key){
    key = key % 16;
    uint8_t row = (15 - key) / 4;
    // return _rows[row];
    return pgm_read_word(&_rows[row]);
}

int Dtmf::col_freq_from_key(uint8_t key){
    key = key % 16;
    uint8_t col = (15 - key) % 4;
    // return _cols[col];
    return pgm_read_word(&_cols[col]);
}
