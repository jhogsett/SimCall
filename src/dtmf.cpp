#include "dtmf.h"

Dtmf::Dtmf(){
    _rows[0] = 697;
    _rows[1] = 770;
    _rows[2] = 852;
    _rows[3] = 941;
    _cols[0] = 1209;
    _cols[1] = 1336;
    _cols[2] = 1477;
    _cols[3] = 1633;
  }

  int Dtmf::row_freq_from_key(int8_t key){
    int8_t row = (15 - key) / 4;
    return _rows[row];
  }

  int Dtmf::col_freq_from_key(int8_t key){
    int8_t col = (15 - key) % 4;
    return _cols[col];
  }


