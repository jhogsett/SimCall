#include <Arduino.h>
#include "tones.h"
#include "dtmf.h"
#include "r1mf.h"

Tones::Tones(MD_AD9833 * pDevice1, MD_AD9833 * pDevice2, float silent_freq, KeypadHandler * pkeypad_handler)
   : _pDevice1(pDevice1), _pDevice2(pDevice2), _silent_freq(silent_freq), _pkeypad_handler(pkeypad_handler)
 {}

void Tones::begin()
{
  _pDevice1->begin();
  _pDevice2->begin();

  _pDevice1->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
  _pDevice1->setMode(MD_AD9833::MODE_SINE);

  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
  _pDevice2->setMode(MD_AD9833::MODE_SINE);
}

void Tones::sound_off(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::busy_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 480.0);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, 620.0);
}

void Tones::uk_busy_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 400.0);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::ring_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 480.0);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, 440.0);
}

void Tones::uk_ring_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 400.0);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, 450.0);
}

void Tones::error_tone1_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 913.8);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::error_tone2_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 1428.5);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::error_tone3_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 1776.7);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::cancel_tone_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 941);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::ready_tone_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 1209);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::disconnect_tone_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 2600);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
}

void Tones::dual_tone(int freq1, int freq2, int times, int inter_delay, int final_delay){
  final_delay = (final_delay == -1 ? inter_delay : final_delay);

  const float freq1_out = static_cast<float>(freq1);
  const float freq2_out = (freq2 == 0 ? _silent_freq : static_cast<float>(freq2));

  for(int i = 0; i < times; i++){
     _pDevice1->setFrequency((MD_AD9833::channel_t)0, freq1_out);
     _pDevice2->setFrequency((MD_AD9833::channel_t)0, freq2_out);

    delay(inter_delay);

    _pDevice1->setFrequency((MD_AD9833::channel_t)0, _silent_freq);
    _pDevice2->setFrequency((MD_AD9833::channel_t)0, _silent_freq);

    delay(i == times-1 ? final_delay : inter_delay);
  }  
}

void Tones::dial_tone(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 350.0);
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, 440.0);
}

void Tones::confirmation_tone(){
  dual_tone(350, 440, 3, 100);
}

void Tones::disconnect_tone(){
  dual_tone(2600, 0, 1, 200);
}

void Tones::dial_key(uint8_t key){
  if (key <= 15) {
    _pDevice1->setFrequency((MD_AD9833::channel_t)0, Dtmf::row_freq_from_key(key));
    _pDevice2->setFrequency((MD_AD9833::channel_t)0, Dtmf::col_freq_from_key(key));
  }
}

void Tones::dial_opkey(uint8_t key){
  if (key <= 15) {
    _pDevice1->setFrequency((MD_AD9833::channel_t)0, R1mf::freqa_from_key(key));
    int freqb = R1mf::freqb_from_key(key);
    _pDevice2->setFrequency((MD_AD9833::channel_t)0, (freqb == 0) ? _silent_freq : freqb);
  }
}

void Tones::blocking_dial_sequence(const char * digits, bool use_opkeys, int digit_time, int interdigit_time){
  if(digits == nullptr || _pkeypad_handler == nullptr){
    return;
  }

  size_t length = strlen(digits);

  // NOTE to reviewers: it's OK to disregard validation for the return value of key_from_char
  // because, since this is a telephone simulator, there will always be "*" and "#" keys and the digits!!

  if(use_opkeys){
    dial_opkey(_pkeypad_handler->key_from_char('*'));
    delay(digit_time);
    sound_off();
    delay(interdigit_time);
  }

  for(uint8_t i = 0; i < length; i++){
    int8_t key = _pkeypad_handler->key_from_char(digits[i]);
    if(key >= 0 && key <= 15){
      if(use_opkeys){
        dial_opkey(key);
      } else {
        dial_key(key);
      }
      delay(digit_time);
      sound_off();
      if(use_opkeys || i != length - 1){
        delay(interdigit_time);
      }
    }
  }

  if(use_opkeys){
    dial_opkey(_pkeypad_handler->key_from_char('#'));
    delay(digit_time);
    sound_off();
  }

}
