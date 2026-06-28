#ifndef SIMCALL_DTMF_H
#define SIMCALL_DTMF_H

#include <Arduino.h>

class Dtmf
{
public:
  // keymap specifies the characters associated with the 4x4 keypad from the bottom right key to the top left key (down->up, right->left)
  // this is the same order the I2CKeyPad expects
  Dtmf();

  int row_freq_from_key(int8_t key);

  int col_freq_from_key(int8_t key);

private:
  int _rows[4];
  int _cols[4];
};

#endif
