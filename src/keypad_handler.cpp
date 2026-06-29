#include "keypad_handler.h"
#include "I2CKeyPad.h"

using KeypadAction = void (*)(int8_t key, char ch);

KeypadHandler::KeypadHandler(I2CKeyPad * pkeypad, const char * pkeymap, int min_keypress_time)
  : _pkeypad(pkeypad), _pkeymap(pkeymap), _min_keypress_time(min_keypress_time), _key_press_state(STATE_IDLE), 
  _key_press_time(0), _pressed_key(-1), _pressed_char('\0')
{}

bool KeypadHandler::begin()
{
  if(!_pkeypad->begin()){
    return false;
  }

  _pkeypad->loadKeyMap(const_cast<char *>(_pkeymap));
  return true;
}

bool KeypadHandler::char_in_chars(char ch, const char * chars){
  const char * chariter = chars;
  while(*chariter != '\0'){
    if(ch == *chariter){
      return true;
    }
    chariter++;
  }
  return false;
}

bool KeypadHandler::keypad_pressed(){
  return _pkeypad->isPressed();
}

KeypadHandler::States KeypadHandler::keypad_event_step(KeypadAction press_action, KeypadAction release_action){
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
      if(release_action != NULL){
          release_action(_pressed_key, _pressed_char);
      }
      break;
  }

  return _key_press_state;
}

bool KeypadHandler::keypad_state_wait(int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action, KeypadAction release_action){
  if(keypad_event_step(press_action, release_action) == state){
      return true;
  }
  return false;
}

char KeypadHandler::keypad_char_wait(const char * valid_chars, int state=STATE_LEGIT_KEY_RELEASE, KeypadAction press_action, KeypadAction release_action){
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

char KeypadHandler::wait_for_char(const char * valid_chars, unsigned long timeoutms=1000, int completion_state=STATE_IDLE, KeypadAction press_action, KeypadAction release_action)
{
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

