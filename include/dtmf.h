#ifndef SIMCALL_DTMF_H
#define SIMCALL_DTMF_H

#include <Arduino.h>

class Dtmf
{
public:
  Dtmf();

  int row_freq_from_key(uint8_t key);

  int col_freq_from_key(uint8_t key);

private:
  int _rows[4];
  int _cols[4];
};

#endif
