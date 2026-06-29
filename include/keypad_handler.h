#ifndef SIMCALL_KEYPAD_HANDLER_H 
#define SIMCALL_KEYPAD_HANDLER_H 

#include <Arduino.h>
#include "I2CKeyPad.h"

using KeypadAction = void (*)(int8_t key, char ch);

class KeypadHandler
{
public:
  enum States : byte {
    STATE_IDLE,
    STATE_MAYBE_KEY_PRESS,
    STATE_LEGIT_KEY_PRESS,
    STATE_CONTINUED_KEY_PRESS,
    STATE_MAYBE_KEY_RELEASE,
    STATE_LEGIT_KEY_RELEASE
  };

  KeypadHandler(I2CKeyPad * pkeypad, const char * keymap, int min_keypress_time);
  bool begin();
  static bool char_in_chars(char ch, const char * chars);
  bool keypad_pressed();
  KeypadHandler::States keypad_event_step(KeypadAction press_action, KeypadAction release_action);
  bool keypad_state_wait(int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action = nullptr, KeypadAction release_action = nullptr);
  char keypad_char_wait(const char * valid_chars, int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action = nullptr, KeypadAction release_action = nullptr);
  char wait_for_char(const char * valid_chars, unsigned long timeoutms=1000, int completion_state=STATE_IDLE, KeypadAction press_action = nullptr, KeypadAction release_action = nullptr);

private:
  I2CKeyPad * _pkeypad;
  const char * _pkeymap;
  int _min_keypress_time;

  States _key_press_state;
  unsigned long _key_press_time;
  int8_t _pressed_key;
  char _pressed_char;
};

#endif
