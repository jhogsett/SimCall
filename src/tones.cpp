#include <Arduino.h>
#include "tones.h"
#include "dtmf.h"

Dtmf Tones::_dtmf;

Tones::Tones(MD_AD9833 * pDevice1, MD_AD9833 * pDevice2, float silent_freq)
{
    _pDevice1 = pDevice1;
    _pDevice2 = pDevice2;
    _silent_freq = silent_freq;
}

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
}

void Tones::error_tone2_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 1428.5);
}

void Tones::error_tone3_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 1776.7);
}

void Tones::cancel_tone_on(){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, 941);
}

void Tones::dual_tone(int freq1, int freq2, int times, int inter_delay, int final_delay){
  final_delay = (final_delay == -1 ? inter_delay : final_delay);
  freq2 = (freq2 == 0 ? _silent_freq : freq2);
  for(int i = 0; i < times; i++){
    _pDevice1->setFrequency((MD_AD9833::channel_t)0, freq1);
    _pDevice2->setFrequency((MD_AD9833::channel_t)0, freq2);
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

void Tones::dial_key(int key){
  _pDevice1->setFrequency((MD_AD9833::channel_t)0, _dtmf.row_freq_from_key(key));
  _pDevice2->setFrequency((MD_AD9833::channel_t)0, _dtmf.col_freq_from_key(key));
}
