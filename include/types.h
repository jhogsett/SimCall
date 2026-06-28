#ifndef SIMCALL_TYPES_H
#define SIMCALL_TYPES_H

#include "I2CKeyPad.h"

using KeypadAction = void (*)(int8_t key, char ch);

class KeypadHandler
{
public:
  KeypadHandler(I2CKeyPad * pkeypad, int min_keypress_time){
    _pkeypad = pkeypad;
    _min_keypress_time = min_keypress_time;    

    // which of these need initializing?
    _key_press_state = STATE_IDLE;
    _key_press_time = 0;
    _pressed_key = -1;
    _pressed_char = '\0';
  }

  static bool char_in_chars(char ch, const char * chars){
    const char * chariter = chars;
    while(*chariter != '\0'){
      if(ch == *chariter){
        return true;
      }
      chariter++;
    }
    return false;
  }

  bool keypad_pressed(){
    return _pkeypad->isPressed();
  }

  int keypad_event_step(KeypadAction press_action, KeypadAction release_action){
    bool key_pressed = keypad_pressed();

    switch(_key_press_state){
      case STATE_IDLE:
        // if key pressed, and it is found to be a real key, start a new key press legitimacy check
        if(key_pressed and _pkeypad->getKey() != I2C_KEYPAD_NOKEY){
          _key_press_state = STATE_MAYBE_KEY_PRESS;
          _key_press_time = millis();
        }
        // otherwise, continue waiting for a key press
        break;

      case STATE_MAYBE_KEY_PRESS:
        // if key pressed, check the time
        if(key_pressed){
          // if key has been pressed for long enough consider it legit
          if(_key_press_time + _min_keypress_time <= millis()){
            _key_press_state = STATE_LEGIT_KEY_PRESS;
          } 
          // otherwise, continue waiting
        } 
        // if key not pressed, it wasn't a legit key press
        else {
            _key_press_state = STATE_IDLE;
        }
        break;

      case STATE_LEGIT_KEY_PRESS:
        // otherwise, act on the key press and continue with a legit key press
        _pressed_key = _pkeypad->getLastKey();
        _pressed_char = _pkeypad->getLastChar();
        _key_press_state = STATE_CONTINUED_KEY_PRESS;
        // Serial.println("legit key press");
        if(press_action != NULL){
          press_action(_pressed_key, _pressed_char);
        }
        break;

      case STATE_CONTINUED_KEY_PRESS:
        // if key released, start a new key release legitimacy check
        if(!key_pressed){
          _key_press_state = STATE_MAYBE_KEY_RELEASE;
          _key_press_time = millis();
        }
        break;

      case STATE_MAYBE_KEY_RELEASE:
        // if key released, check the time
        if(!key_pressed){
          // if key has been released for long enough consider it legit
          if(_key_press_time + _min_keypress_time <= millis()){
            _key_press_state = STATE_LEGIT_KEY_RELEASE;
          } 
          // otherwise, continue waiting
        } 
        // if key not released, it wasn't a legit key release
        else {
            _key_press_state = STATE_CONTINUED_KEY_PRESS;
        }
        break;

      case STATE_LEGIT_KEY_RELEASE:
        // otherwise, act on the key release and continue with a legit key relase (idle)
        _pressed_key = I2C_KEYPAD_NOKEY;
        _key_press_state = STATE_IDLE;
        // Serial.println("legit key release");
        if(release_action != NULL){
          release_action(_pressed_key, _pressed_char);
        }
        break;
    }

    return _key_press_state;
  }

  bool keypad_state_wait(int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action=NULL, KeypadAction release_action=NULL){
    if(keypad_event_step(press_action, release_action) == state){
      return true;
    }
    return false;
  }

  char keypad_char_wait(const char * valid_chars, int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action=NULL, KeypadAction release_action=NULL){
    if(!keypad_state_wait(state, press_action, release_action)){
      return '\0';
    }
    if(valid_chars == NULL){
      return _pkeypad->getLastChar();
    } else {
      const char * chariter = valid_chars;
      char ch = _pkeypad->getLastChar();
      while(*chariter != '\0'){
        if(ch == *chariter){
          return ch;
        }
        chariter++;
      }
    }
    return '\0';
  }

  char wait_for_char(const char * valid_chars, unsigned long timeoutms=1000, int completion_state=STATE_IDLE, KeypadAction press_action=NULL, KeypadAction release_action=NULL){
    unsigned long timeout_time = millis() + timeoutms;
    char ch;
    while(millis() < timeout_time){
      // should this be STATE_CONTINUED_KEY_PRESS?
      // ch = keypad_char_wait(valid_chars, STATE_LEGIT_KEY_PRESS, press_action, release_action);
      ch = keypad_char_wait(valid_chars, STATE_CONTINUED_KEY_PRESS, press_action, release_action);
      if(ch != '\0'){
        // should the timeout apply here or get stuck forever on a pressed key?
        while(millis() < timeout_time){
          // actions have to be specified here, they're not call on the keypad_char_wait() line (?)
          if(keypad_state_wait(completion_state, press_action, release_action)){
          // if(keypad_state_wait(completion_state, '\0', '\0')){
            return ch;
          }
        }
      }
    }
    return '\0';
  }

  enum State : byte {
    STATE_IDLE,
    STATE_MAYBE_KEY_PRESS,
    STATE_LEGIT_KEY_PRESS,
    STATE_CONTINUED_KEY_PRESS,
    STATE_MAYBE_KEY_RELEASE,
    STATE_LEGIT_KEY_RELEASE
  };

private:
  I2CKeyPad * _pkeypad;
  int _min_keypress_time;

  int _key_press_state;
  unsigned long _key_press_time;
  int8_t _pressed_key;
  char _pressed_char;
};


using NonBlockingAction = void (*)(uint32_t data);
using NonBlockingAbort = bool (*)(uint32_t data);

class NonBlockingTimer
{
public:
  void start(uint32_t duration)
  {
    _start_time = millis();
    _duration = duration;
  }

  bool elapsed()
  {
    return (millis() - _start_time) >= _duration;
  }

private:
  uint32_t _start_time;
  uint32_t _duration;
};

class NonBlockingLoop
{
public:
  void start(uint32_t cycles)
  {
    _cycles = cycles;
    _cycle = 0;
  }

  bool active()
  {
    return _cycle < _cycles;
  }

  bool last_cycle()
  {
    return _cycle == _cycles - 1;
  }

  bool cycle()
  {
    _cycle += 1;
    return active();
  }

  uint32_t cycle() const { return _cycle; }

private:
  uint32_t _cycles;
  uint32_t _cycle;
};

class NonBlockingSequence
{
public:
  NonBlockingSequence(NonBlockingAction* actions, int* times, int num_actions, bool quick_last_action = false, NonBlockingAbort abort_action = NULL){
    _actions = actions;
    _times = times;
    _num_actions = num_actions;
    _abort_action = abort_action;
    _quick_last_action = quick_last_action;
  }

  void start(int num_cycles, uint32_t data = 0L){
    _num_cycles = num_cycles;
    _data = data;
    restart();
  }

  void restart(){
    _action_iter = 0;
    _state = STATE_START_LOOP;
    _running = false;
    _aborted = false;
  }

  bool aborted(){
    return _aborted;
  }

  bool quick_last_action(){
    return _quick_last_action && _loop.last_cycle() && _action_iter == _num_actions - 1;
  }

  bool step(){
    if(_abort_action && _abort_action(0)){
      _state = STATE_ABORTED;
    }

    switch(_state){
      case STATE_START_LOOP:
        _loop.start(_num_cycles);
        _running = true;
        _state = STATE_START_CYCLE;
        break;

      case STATE_START_CYCLE:
        _action_iter = 0;
        _state = STATE_START_ACTION;
        break;

      case STATE_START_ACTION:
        _action = _actions[_action_iter];
        if(_action != NULL){
          _action(_data);
        }

        // end the loop if requested to skip last cycle and last action timing 
        if(quick_last_action()){
          _state = STATE_END_LOOP;
          break;
        }

        _time = _times[_action_iter];
        _timer.start(_time);
        _state = STATE_RUNNING;
        break;      

      case STATE_RUNNING:
        if(_timer.elapsed()){
          _state = STATE_NEXT_ACTION;
        }
        break;

      case STATE_NEXT_ACTION:
        _action_iter += 1;
        _state = (_action_iter >= _num_actions) ? STATE_NEXT_CYCLE : STATE_START_ACTION;
        break;

      case STATE_NEXT_CYCLE:
        _state = _loop.cycle() ? STATE_START_CYCLE : STATE_END_LOOP;
        break;

      case STATE_END_LOOP:
        _running = false;
        break;

      case STATE_ABORTED:
        _aborted = true;
        _running = false;
        break;
    }

    return _running;
  }

  enum State : byte {
    STATE_START_LOOP,
    STATE_START_CYCLE,
    STATE_START_ACTION,
    STATE_RUNNING,
    STATE_NEXT_ACTION,
    STATE_NEXT_CYCLE,
    STATE_END_LOOP,
    STATE_ABORTED
  };

private:
  NonBlockingAction* _actions;
  int* _times;
  int _num_actions;
  int _num_cycles;
  uint32_t _data;
  int _action_iter;
  NonBlockingAbort _abort_action;
  bool _quick_last_action;

  State _state;
  NonBlockingLoop _loop;
  NonBlockingAction _action;
  int _time;
  NonBlockingTimer _timer;
  bool _running;
  bool _aborted;
};

#endif