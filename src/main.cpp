#include <Arduino.h>
#include "Wire.h"
#include <SPI.h>
#include "I2CKeyPad.h"
#include <MD_AD9833.h>
#include "types.h"
#include <random_seed.h>
#include "hook_light.h"
#include "tones.h"

// Pins for SPI comm with the AD9833 IC
const uint8_t PIN_DATA = 11;  ///< SPI Data pin number
const uint8_t PIN_CLK = 13;  	///< SPI Clock pin number
const uint8_t PIN_FSYNC1 = A2; //8; ///< SPI Load pin number (FSYNC in AD9833 usage)
const uint8_t PIN_FSYNC2 = A3; //7;  ///< SPI Load pin number (FSYNC in AD9833 usage)

const uint8_t RANDOM_SEED_PIN = A0;
static RandomSeed<RANDOM_SEED_PIN> randomizer;

const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS);
char keymap[20] = "D#0*C987B654A321INF";  //  N = NoKey, F = Fail

MD_AD9833	AD1(PIN_DATA, PIN_CLK, PIN_FSYNC1); // Arbitrary SPI pins
MD_AD9833	AD2(PIN_DATA, PIN_CLK, PIN_FSYNC2); // Arbitrary SPI pins

const uint8_t MIN_KEYPRESS_TIME = 20;
KeypadHandler keypad_handler(&keyPad, MIN_KEYPRESS_TIME);

const uint8_t HOOK_LIGHT_PIN = A1;
HookLight hook_light(HOOK_LIGHT_PIN);

const float SILENT_FREQ = 50000.0;
Tones tones(&AD1, &AD2, SILENT_FREQ);

bool begin_keypad(I2CKeyPad& keypad, const char * keymap, const uint8_t address = 0x20){
  if(!keyPad.begin()){
    return false;
  }

  keyPad.loadKeyMap(const_cast<char *>(keymap));
  return true;
}

void sound_off(uint32_t data){
  tones.sound_off();
}

void busy_on(uint32_t data){
  tones.busy_on();
}

void uk_busy_on(uint32_t data){
  tones.uk_busy_on();
}

void ring_on(uint32_t data){
  tones.ring_on();
}

void uk_ring_on(uint32_t data){
  tones.uk_ring_on();
}

void error_tone1_on(uint32_t data){
  tones.error_tone1_on();
}

void error_tone2_on(uint32_t data){
  tones.error_tone2_on();
}

void error_tone3_on(uint32_t data){
  tones.error_tone3_on();
}

void cancel_tone_on(uint32_t data){
  tones.cancel_tone_on();
}

NonBlockingAction ring_actions[2] = { ring_on, sound_off};
int ring_times[2] = { 2000, 4000 };
NonBlockingSequence ring_sequence(ring_actions, ring_times, 2, true);

NonBlockingAction uk_ring_actions[4] = { uk_ring_on, sound_off, uk_ring_on, sound_off};
int uk_ring_times[4] = { 400, 200, 400, 2000 };
NonBlockingSequence uk_ring_sequence(uk_ring_actions, uk_ring_times, 4, true);

NonBlockingAction busy_actions[2] = { busy_on, sound_off};
int busy_times[2] = { 500, 500 };
NonBlockingSequence busy_sequence(busy_actions, busy_times, 2, true);

NonBlockingAction uk_busy_actions[2] = { uk_busy_on, sound_off};
int uk_busy_times[2] = { 375, 375 };
NonBlockingSequence uk_busy_sequence(uk_busy_actions, uk_busy_times, 2, true);

NonBlockingAction reorder_actions[2] = { busy_on, sound_off};
int reorder_times[2] = { 250, 250 };
NonBlockingSequence reorder_sequence(reorder_actions, reorder_times, 2, true);

NonBlockingAction uk_reorder_actions[4] = { uk_busy_on, sound_off, uk_busy_on, sound_off};
int uk_reorder_times[4] = { 400, 350, 225, 525 };
NonBlockingSequence uk_reorder_sequence(uk_reorder_actions, uk_reorder_times, 4, true);

NonBlockingAction error_actions[4] = { error_tone1_on, error_tone2_on, error_tone3_on, sound_off};
int error_times[4] = { 380, 276, 380, 0 };
NonBlockingSequence error_sequence(error_actions, error_times, 4, false);

NonBlockingAction uk_error_actions[1] = { uk_busy_on };
int uk_error_times[1] = { 1000 };
NonBlockingSequence uk_error_sequence(uk_error_actions, uk_error_times, 1, false);

NonBlockingAction cancel_actions[4] = { cancel_tone_on, sound_off, cancel_tone_on, sound_off};
int cancel_times[4] = { 50, 50, 50, 50 };
NonBlockingSequence cancel_sequence(cancel_actions, cancel_times, 4, false);

void pop(){
  hook_light.wink();
  tones.dual_tone(200, 200, 1, 7, 0);
  hook_light.wink();
}

void click(){
  hook_light.wink();
  tones.dual_tone(600, 600, 1, 3, 0);
  hook_light.wink();
}

void disconnect_tone(){
  tones.dual_tone(2600, 0, 1, 200);
}

void cancel_tone(){
  cancel_sequence.start(1);
  while(cancel_sequence.step());
}

void error_tone(){
  delay(200);
  error_sequence.start(1);
  while(error_sequence.step());
}

void startup_sequence(){
  delay(500);
  hook_light.on();
  tones.confirmation_tone();
  hook_light.off();
}

void setup() {
  randomizer.randomize();
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);

  if(!begin_keypad(keyPad, keymap)){
    Serial.println("Failed to begin keypad");
  }

  tones.begin();

  pinMode(HOOK_LIGHT_PIN, OUTPUT);
  digitalWrite(HOOK_LIGHT_PIN, HIGH);

  startup_sequence();
}

const uint8_t MAX_DIGITS = 16;
char digits[MAX_DIGITS+1];
int8_t num_digits = 0;

const uint8_t CALL_NONE = 0;
// 867-5209
const uint8_t CALL_LOCAL = 1;
const uint8_t LOCAL_COUNT = 7;
// 1-800-555-1212
const uint8_t CALL_LONG = 2;
const char LONG_PREFIX = '1';
const uint8_t LONG_COUNT = 11;
// 011-44-8302-1212
const uint8_t CALL_INTL = 3;
const char INTL_PREFIX = '0';
const uint8_t INTL_COUNT = 13;

int digit_count = 0;
int call_type = CALL_NONE;

enum Outcomes{
  OUTCOME_RING,
  OUTCOME_BUSY,
  OUTCOME_REORDER,
  OUTCOME_ERROR
};

Outcomes outcome;

int determine_outcome(const char * pressed_digits, int8_t num_digits){
  switch(pressed_digits[num_digits-1]){
    case '1':
      return outcome = OUTCOME_BUSY;
    case '2':
      return outcome = OUTCOME_REORDER;
    case '3':
      return outcome = OUTCOME_ERROR;
  }

  int r = random(0, 1000);

  if(call_type == CALL_INTL){
    if(r < 580){
      outcome = OUTCOME_RING;
    } else if(r < 830){
      outcome = OUTCOME_BUSY;
    } else if(r < 950){
      outcome = OUTCOME_REORDER;
    } else {
      outcome = OUTCOME_ERROR;
    }
  } else if(call_type == CALL_LONG)
  {
    if(r < 680){
      outcome = OUTCOME_RING;
    } else if(r < 910){
      outcome = OUTCOME_BUSY;
    } else if(r < 970){
      outcome = OUTCOME_REORDER;
    } else {
      outcome = OUTCOME_ERROR;
    }
  } else {
    if(r < 730){
      outcome = OUTCOME_RING;
    } else if(r < 980){
      outcome = OUTCOME_BUSY;
    } else if(r < 985){
      outcome = OUTCOME_REORDER;
    } else {
      outcome = OUTCOME_ERROR;
    }
  }
  return outcome;
}

void start_outcome(int outcome){
  if(call_type == CALL_INTL){
    switch(outcome){
      case OUTCOME_RING:
        uk_ring_sequence.start(8);
        break;
      case OUTCOME_BUSY:
        uk_busy_sequence.start(12);
        break;
      case OUTCOME_REORDER:
        uk_reorder_sequence.start(24);
        break;
      case OUTCOME_ERROR:
        uk_error_sequence.start(10);
        break;
    }
  } else {
    switch(outcome){
      case OUTCOME_RING:
        ring_sequence.start(6);
        break;
      case OUTCOME_BUSY:
        busy_sequence.start(12);
        break;
      case OUTCOME_REORDER:
        reorder_sequence.start(24);
        break;
      case OUTCOME_ERROR:
        error_sequence.start(1);
        break;
    }
  }
}

bool step_outcome(int outcome){
  bool keep_going;

  if(call_type == CALL_INTL){
    switch(outcome){
      case OUTCOME_RING:
        keep_going = uk_ring_sequence.step();
        break;
      case OUTCOME_BUSY:
        keep_going = uk_busy_sequence.step();
        break;
      case OUTCOME_REORDER:
        keep_going = uk_reorder_sequence.step();
        break;
      case OUTCOME_ERROR:
        keep_going = uk_error_sequence.step();
        break;
      default:
        keep_going = false;
    }
  } else {
    switch(outcome){
      case OUTCOME_RING:
        keep_going = ring_sequence.step();
        break;
      case OUTCOME_BUSY:
        keep_going = busy_sequence.step();
        break;
      case OUTCOME_REORDER:
        keep_going = reorder_sequence.step();
        break;
      case OUTCOME_ERROR:
        keep_going = error_sequence.step();
        break;
      default:
        keep_going = false;
    }
  }
  return keep_going;
}

void pre_routing_sound(){
  delay(random(500, 1000));
  click();
  delay(random(500, 1000));
  pop();
  delay(300);
}

void post_routing_sound(){
  delay(500);
  pop();
}

void action_dial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    tones.dial_key(key);
  }
}

void action_undial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    sound_off(0);
  }
}

void action_dtmf(int8_t key, char ch){
  tones.dial_key(key);
}

void action_undtmf(int8_t key, char ch){
  sound_off(0);
}

void reset_call(){
  num_digits = 0;
  digit_count = 0;
  call_type = CALL_NONE;
}

void add_digit(char ch){
  if(num_digits < MAX_DIGITS){
    digits[num_digits++] = ch;
    digits[num_digits] = '\0'; // stringify
  }
}

// maybe update routing as more digits come in, 911 etc.

void determine_routing(char ch){
  switch(ch){
    case LONG_PREFIX:
      call_type = CALL_LONG;
      digit_count = LONG_COUNT;
      break;
    case INTL_PREFIX:
      call_type = CALL_INTL;
      digit_count = INTL_COUNT;
      break;
    default:
      call_type = CALL_LOCAL;
      digit_count = LOCAL_COUNT;
      break;
  }
}

enum Modes{
  MODE_WAITING,
  MODE_INITIATE_CALL,
  MODE_CALL_START,
  MODE_CALL_IN_PROGRESS,
  MODE_ROUTING_START,
  MODE_ROUTING_IN_PROGRESS,
  MODE_COMMAND_B,
  MODE_COMMAND_C,
  MODE_COMMAND_D
};

Modes mode = MODE_WAITING;

void loop()
{
  char ch;
  switch(mode){
    case MODE_WAITING:
      if('\0' != (ch = keypad_handler.wait_for_char("ABCD", 1000, KeypadHandler::STATE_CONTINUED_KEY_PRESS, NULL, NULL))){
        switch(ch){
          case 'A':
            mode = MODE_INITIATE_CALL;
            break;
          case 'B':
            mode = MODE_COMMAND_B;
            break;
          case 'C':
            mode = MODE_COMMAND_C;
            break;
          case 'D':
            mode = MODE_COMMAND_D;
            break;
        }
      }
      break;
    case MODE_INITIATE_CALL:
      hook_light.on();
      tones.dial_tone();
      mode = MODE_CALL_START;
      // edge triggered key may still be pressed
      while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
      break;

    case MODE_CALL_START:
      reset_call();
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          cancel_tone();
          mode = MODE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          error_tone();
          delay(200);
          hook_light.off();
          cancel_tone();
          mode = MODE_WAITING;
        } 
        else {
          add_digit(ch);
          determine_routing(ch);
          mode = MODE_CALL_IN_PROGRESS;
        }
      }
      break;

    case MODE_CALL_IN_PROGRESS:
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          cancel_tone();
          mode = MODE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          error_tone();
          delay(200);
          hook_light.off();
          cancel_tone();
          mode = MODE_WAITING;
        } else {
          add_digit(ch);
          if(num_digits >= digit_count){
            mode = MODE_ROUTING_START;
          }
        }
      }
      break;

    case MODE_ROUTING_START:
        start_outcome(determine_outcome(digits, num_digits));
        pre_routing_sound();
        mode = MODE_ROUTING_IN_PROGRESS; 
      break;

    case MODE_ROUTING_IN_PROGRESS:
      if(keypad_handler.keypad_pressed()){
        sound_off(0);
        // edge triggered key may still be pressed
        while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
        hook_light.off();
        cancel_tone();
        mode = MODE_WAITING;
      }
      if(!step_outcome(outcome)){
        sound_off(0);
        post_routing_sound();
        hook_light.off();
       mode = MODE_WAITING;
      }
      break;

    case MODE_COMMAND_B:
    case MODE_COMMAND_C:
    case MODE_COMMAND_D:
      delay(200);
      disconnect_tone();
      mode = MODE_WAITING;
      break;
  }
}
