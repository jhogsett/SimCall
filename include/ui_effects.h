#ifndef SIMCALL_UI_EFFECTS_H
#define SIMCALL_UI_EFFECTS_H

#include "tones.h"
#include "audio_sequences.h"
#include "hook_light.h"

class UIEffects
{
public:
  UIEffects(Tones * ptones, AudioSequences * paudio_sequences, HookLight * phook_light);

  // move these to their own class
  void pop();
  void click();
  void startup_sequence();
  void blocking_cancel_tone();
  void blocking_error_tone();
  void blocking_pre_routing_sound();
  void blocking_post_routing_sound();

private:

  Tones * _ptones;
  AudioSequences * _paudio_sequences;
  HookLight * _phook_light;
};

#endif