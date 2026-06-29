#include "audio_sequences.h"

Tones* AudioSequences::_pTones = nullptr;

void AudioSequences::init(Tones* pTones)
{
  _pTones = pTones;
}

void AudioSequences::_sound_off(uint32_t data){
  _pTones->sound_off();
}

void AudioSequences::_ring_on(uint32_t data){
  _pTones->ring_on();
}

void AudioSequences::_busy_on(uint32_t data){
  _pTones->busy_on();  
}  

void AudioSequences::_uk_ring_on(uint32_t data){
  _pTones->uk_ring_on();
}

void AudioSequences::_uk_busy_on(uint32_t data){
  _pTones->uk_busy_on();      
}  

void AudioSequences::_error_tone1_on(uint32_t data){
  _pTones->error_tone1_on();
}

void AudioSequences::_error_tone2_on(uint32_t data){
  _pTones->error_tone2_on();
}

void AudioSequences::_error_tone3_on(uint32_t data){
  _pTones->error_tone3_on();
}

void AudioSequences::_cancel_tone_on(uint32_t data){
  _pTones->cancel_tone_on();
}

NonBlockingAction AudioSequences::_ring_actions[] = { AudioSequences::_ring_on, AudioSequences::_sound_off};
int AudioSequences::_ring_times[2] { 2000, 4000 };
NonBlockingSequence AudioSequences::ring_sequence(_ring_actions, _ring_times, 2, true);

NonBlockingAction AudioSequences::_uk_ring_actions[4] = { AudioSequences::_uk_ring_on, AudioSequences::_sound_off, AudioSequences::_uk_ring_on, AudioSequences::_sound_off};
int AudioSequences::_uk_ring_times[4] = { 400, 200, 400, 2000 };
NonBlockingSequence AudioSequences::uk_ring_sequence(_uk_ring_actions, _uk_ring_times, 4, true);

NonBlockingAction AudioSequences::_busy_actions[2] = { AudioSequences::_busy_on, AudioSequences::_sound_off};
int AudioSequences::_busy_times[2] = { 500, 500 };
NonBlockingSequence AudioSequences::busy_sequence(_busy_actions, _busy_times, 2, true);

NonBlockingAction AudioSequences::_uk_busy_actions[2] = { AudioSequences::_uk_busy_on, AudioSequences::_sound_off};
int AudioSequences::_uk_busy_times[2] = { 375, 375 };
NonBlockingSequence AudioSequences::uk_busy_sequence(_uk_busy_actions, _uk_busy_times, 2, true);

NonBlockingAction AudioSequences::_reorder_actions[2] = { AudioSequences::_busy_on, AudioSequences::_sound_off};
int AudioSequences::_reorder_times[2] = { 250, 250 };
NonBlockingSequence AudioSequences::reorder_sequence(_reorder_actions, _reorder_times, 2, true);

NonBlockingAction AudioSequences::_uk_reorder_actions[4] = { AudioSequences::_uk_busy_on,AudioSequences::_sound_off, AudioSequences::_uk_busy_on, AudioSequences::_sound_off};
int AudioSequences::_uk_reorder_times[4] = { 400, 350, 225, 525 };
NonBlockingSequence AudioSequences::uk_reorder_sequence(_uk_reorder_actions, _uk_reorder_times, 4, true);

NonBlockingAction AudioSequences::_error_actions[4] = { AudioSequences::_error_tone1_on, AudioSequences::_error_tone2_on, AudioSequences::_error_tone3_on, AudioSequences::_sound_off};
int AudioSequences::_error_times[4] = { 380, 276, 380, 0 };
NonBlockingSequence AudioSequences::error_sequence(_error_actions, _error_times, 4, false);

NonBlockingAction AudioSequences::_uk_error_actions[1] = { AudioSequences::_uk_busy_on };
int AudioSequences::_uk_error_times[1] = { 1000 };
NonBlockingSequence AudioSequences::uk_error_sequence(_uk_error_actions, _uk_error_times, 1, false);

NonBlockingAction AudioSequences::_cancel_actions[4] = { AudioSequences::_cancel_tone_on, AudioSequences::_sound_off, AudioSequences::_cancel_tone_on, AudioSequences::_sound_off};
int AudioSequences::_cancel_times[4] = { 50, 50, 50, 50 };
NonBlockingSequence AudioSequences::cancel_sequence(_cancel_actions, _cancel_times, 4, false);
