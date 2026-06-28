#ifndef SIMCALL_AUDIO_SEQUENCES_H
#define SIMCALL_AUDIO_SEQUENCES_H

#include <Arduino.h>
#include "types.h"
#include "tones.h"

class AudioSequences
{
public:
  AudioSequences(Tones * pTones);

  static NonBlockingSequence ring_sequence;

private:
  static Tones * _pTones;

  static void _ring_on(uint32_t data);

  static void _sound_off(uint32_t data);

  static NonBlockingAction _ring_actions[2];
  static int _ring_times[2];


};




#endif
