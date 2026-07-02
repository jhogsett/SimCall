#include "ui_effects.h"
#include "audio_sequences.h"

UIEffects::UIEffects(Tones * ptones, HookLight * phook_light) 
  : _ptones(ptones), _phook_light(phook_light)
{}

void UIEffects::pop(bool sound_only){
  if(!sound_only) _phook_light->wink();
  _ptones->dual_tone(200, 200, 1, 7, 0);
  if(!sound_only) _phook_light->wink();
}

void UIEffects::click(bool sound_only){
  if(!sound_only) _phook_light->wink();
  _ptones->dual_tone(600, 600, 1, 3, 0);
  if(!sound_only) _phook_light->wink();
}

void UIEffects::startup_sequence(){
  delay(500);
  _phook_light->on();
  _ptones->confirmation_tone();
  _phook_light->off();
}

void UIEffects::blocking_ready_tone(){
  AudioSequences::ready_sequence.start(1);
  while(AudioSequences::ready_sequence.step()){
    delay(1);
  }
}

void UIEffects::blocking_cancel_tone(){
  AudioSequences::cancel_sequence.start(1);
  while(AudioSequences::cancel_sequence.step()){
    delay(1);
  }
}

void UIEffects::blocking_error_tone(){
  delay(200);
  AudioSequences::error_sequence.start(1);
  while(AudioSequences::error_sequence.step()){
    delay(1);
  }
}

void UIEffects::blocking_pre_routing_sound(){
  delay(random(500, 1000));
  click();
  delay(random(500, 1000));
  pop();
  delay(300);
}

void UIEffects::blocking_post_routing_sound(){
  delay(500);
  pop();
}

void UIEffects::blocking_disconnect(){
  delay(500);
  AudioSequences::disconnect_sequence.start(1);
  while(AudioSequences::disconnect_sequence.step()){
    delay(1);
  }
  delay(500);
}

void UIEffects::blocking_wink(){
  click(true);
  _phook_light->wink();
  delay(200);
  _phook_light->wink();
  pop(true);
  delay(200);
}

