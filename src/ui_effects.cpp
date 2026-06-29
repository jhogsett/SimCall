#include "ui_effects.h"

UIEffects::UIEffects(Tones * ptones, AudioSequences * paudio_sequences, HookLight * phook_light) 
  : _ptones(ptones), _paudio_sequences(paudio_sequences), _phook_light(phook_light)
{}

// move these to their own class
void UIEffects::pop(){
  _phook_light->wink();
  _ptones->dual_tone(200, 200, 1, 7, 0);
  _phook_light->wink();
}

void UIEffects::click(){
  _phook_light->wink();
  _ptones->dual_tone(600, 600, 1, 3, 0);
  _phook_light->wink();
}

void UIEffects::startup_sequence(){
  delay(500);
  _phook_light->on();
  _ptones->confirmation_tone();
  _phook_light->off();
}

void UIEffects::blocking_cancel_tone(){
  _paudio_sequences->cancel_sequence.start(1);
  while(_paudio_sequences->cancel_sequence.step());
}

void UIEffects::blocking_error_tone(){
  delay(200);
  _paudio_sequences->error_sequence.start(1);
  while(_paudio_sequences->error_sequence.step());
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
