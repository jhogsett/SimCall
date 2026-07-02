#ifndef SIMCALL_UI_EFFECTS_H
#define SIMCALL_UI_EFFECTS_H

#include "tones.h"
#include "audio_sequences.h"
#include "hook_light.h"

class UIEffects
{
public:
  UIEffects(Tones * ptones, HookLight * phook_light);

  void pop(bool sound_only=false);
  void click(bool sound_only=false);
  void startup_sequence();
  void blocking_ready_tone();
  void blocking_cancel_tone();
  void blocking_error_tone();
  void blocking_pre_routing_sound();
  void blocking_post_routing_sound();
  void blocking_disconnect();
  void blocking_wink();

private:

  Tones * _ptones;
  HookLight * _phook_light;
};

#endif