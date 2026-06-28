#include "audio_sequences.h"

Tones* AudioSequences::_pTones = nullptr;

AudioSequences::AudioSequences(Tones * pTones)
{
  _pTones = pTones;
}

void AudioSequences::_sound_off(uint32_t data){
  _pTones->sound_off();
}

// void busy_on(uint32_t data){
//   tones.busy_on();
// }

// void uk_busy_on(uint32_t data){
//   tones.uk_busy_on();
// }

void AudioSequences::_ring_on(uint32_t data){
  _pTones->ring_on();
}

NonBlockingAction AudioSequences::_ring_actions[] = { _ring_on, _sound_off};
int AudioSequences::_ring_times[2] { 2000, 4000 };
NonBlockingSequence AudioSequences::ring_sequence(_ring_actions, _ring_times, 2, true);


// void uk_ring_on(uint32_t data){
//   tones.uk_ring_on();
// }

// void error_tone1_on(uint32_t data){
//   tones.error_tone1_on();
// }

// void error_tone2_on(uint32_t data){
//   tones.error_tone2_on();
// }

// void error_tone3_on(uint32_t data){
//   tones.error_tone3_on();
// }

// void cancel_tone_on(uint32_t data){
//   tones.cancel_tone_on();
// }

// NonBlockingAction ring_actions[2] = { ring_on, sound_off};
// int ring_times[2] = { 2000, 4000 };
// NonBlockingSequence ring_sequence(ring_actions, ring_times, 2, true);

// NonBlockingAction uk_ring_actions[4] = { uk_ring_on, sound_off, uk_ring_on, sound_off};
// int uk_ring_times[4] = { 400, 200, 400, 2000 };
// NonBlockingSequence uk_ring_sequence(uk_ring_actions, uk_ring_times, 4, true);

// NonBlockingAction busy_actions[2] = { busy_on, sound_off};
// int busy_times[2] = { 500, 500 };
// NonBlockingSequence busy_sequence(busy_actions, busy_times, 2, true);

// NonBlockingAction uk_busy_actions[2] = { uk_busy_on, sound_off};
// int uk_busy_times[2] = { 375, 375 };
// NonBlockingSequence uk_busy_sequence(uk_busy_actions, uk_busy_times, 2, true);

// NonBlockingAction reorder_actions[2] = { busy_on, sound_off};
// int reorder_times[2] = { 250, 250 };
// NonBlockingSequence reorder_sequence(reorder_actions, reorder_times, 2, true);

// NonBlockingAction uk_reorder_actions[4] = { uk_busy_on, sound_off, uk_busy_on, sound_off};
// int uk_reorder_times[4] = { 400, 350, 225, 525 };
// NonBlockingSequence uk_reorder_sequence(uk_reorder_actions, uk_reorder_times, 4, true);

// NonBlockingAction error_actions[4] = { error_tone1_on, error_tone2_on, error_tone3_on, sound_off};
// int error_times[4] = { 380, 276, 380, 0 };
// NonBlockingSequence error_sequence(error_actions, error_times, 4, false);

// NonBlockingAction uk_error_actions[1] = { uk_busy_on };
// int uk_error_times[1] = { 1000 };
// NonBlockingSequence uk_error_sequence(uk_error_actions, uk_error_times, 1, false);

// NonBlockingAction cancel_actions[4] = { cancel_tone_on, sound_off, cancel_tone_on, sound_off};
// int cancel_times[4] = { 50, 50, 50, 50 };
// NonBlockingSequence cancel_sequence(cancel_actions, cancel_times, 4, false);
