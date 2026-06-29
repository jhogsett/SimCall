#ifndef SIMCALL_DTMF_H
#define SIMCALL_DTMF_H

#include <Arduino.h>

class Dtmf
{
public:
  static int row_freq_from_key(uint8_t key);
  static int col_freq_from_key(uint8_t key);

private:
  static constexpr int _rows[4] = { 697, 770, 852, 941 };
  static constexpr int _cols[4] = { 1209, 1336, 1477, 1633 };
};

#endif
