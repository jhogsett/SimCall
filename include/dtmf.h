#ifndef SIMCALL_DTMF_H
#define SIMCALL_DTMF_H

#include <Arduino.h>

class Dtmf
{
public:
  static int row_freq_from_key(uint8_t key);
  static int col_freq_from_key(uint8_t key);

private:
  static const int _rows[4] PROGMEM;
  static const int _cols[4] PROGMEM;
};

#endif
