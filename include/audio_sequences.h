#ifndef SIMCALL_AUDIO_SEQUENCES_H
#define SIMCALL_AUDIO_SEQUENCES_H

#include <Arduino.h>
#include "types.h"
#include "tones.h"

class AudioSequences
{
public:
  static void init(Tones* pTones); 

  static NonBlockingSequence ring_sequence;
  static NonBlockingSequence busy_sequence;
  static NonBlockingSequence reorder_sequence;
  static NonBlockingSequence error_sequence;
  static NonBlockingSequence uk_ring_sequence;
  static NonBlockingSequence uk_busy_sequence;
  static NonBlockingSequence uk_reorder_sequence;
  static NonBlockingSequence uk_error_sequence;
  static NonBlockingSequence ready_sequence;
  static NonBlockingSequence cancel_sequence;
  static NonBlockingSequence disconnect_sequence;

private:
  static Tones * _pTones;

  static void _sound_off(uint32_t data);
  static void _ring_on(uint32_t data);
  static void _busy_on(uint32_t data);
  static void _uk_ring_on(uint32_t data);
  static void _uk_busy_on(uint32_t data);
  static void _error_tone1_on(uint32_t data);
  static void _error_tone2_on(uint32_t data);
  static void _error_tone3_on(uint32_t data);
  static void _cancel_tone_on(uint32_t data);
  static void _disconnect_tone_on(uint32_t data);

  static NonBlockingAction _ring_actions[2];
  static int _ring_times[2];

  static NonBlockingAction _busy_actions[2];
  static int _busy_times[2];

  static NonBlockingAction _reorder_actions[2];
  static int _reorder_times[2];
  
  static NonBlockingAction _error_actions[4];
  static int _error_times[4];

  static NonBlockingAction _uk_ring_actions[4];
  static int _uk_ring_times[4];
  
  static NonBlockingAction _uk_busy_actions[2];
  static int _uk_busy_times[2];
  
  static NonBlockingAction _uk_reorder_actions[4];
  static int _uk_reorder_times[4];
  
  static NonBlockingAction _uk_error_actions[1];
  static int _uk_error_times[1];
  
  static NonBlockingAction _ready_actions[2];
  static int _ready_times[2];

  static NonBlockingAction _cancel_actions[4];
  static int _cancel_times[4];

  static NonBlockingAction _disconnect_actions[2];
  static int _disconnect_times[2];

};

#endif
