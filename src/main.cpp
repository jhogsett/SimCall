#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "I2CKeyPad.h"
#include <MD_AD9833.h>
#include <random_seed.h>
#include "types.h"
#include "hook_light.h"
#include "tones.h"
#include "audio_sequences.h"
#include "keypad_handler.h"
#include "ui_effects.h"

constexpr uint8_t RANDOM_SEED_PIN = A0;
static RandomSeed<RANDOM_SEED_PIN> randomizer;

const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS);
char keymap[20] = "D#0*C987B654A321INF"; // I - Invalid?, N = NoKey, F = Fail
const uint8_t MIN_KEYPRESS_TIME = 20;
KeypadHandler keypad_handler(&keyPad, keymap, MIN_KEYPRESS_TIME);

const uint8_t PIN_DATA = 11;
const uint8_t PIN_CLK = 13;
const uint8_t PIN_FSYNC1 = A2;
const uint8_t PIN_FSYNC2 = A3;
MD_AD9833	AD1(PIN_DATA, PIN_CLK, PIN_FSYNC1);
MD_AD9833	AD2(PIN_DATA, PIN_CLK, PIN_FSYNC2);

const float SILENT_FREQ = 50000.0;
Tones tones(&AD1, &AD2, SILENT_FREQ);

const uint8_t HOOK_LIGHT_PIN = A1;
HookLight hook_light(HOOK_LIGHT_PIN);

UIEffects ui_effects(&tones, &hook_light);

void setup() {
  Serial.begin(115200);
  randomizer.randomize();

  Wire.begin();
  Wire.setClock(400000);

  if(!keypad_handler.begin()){
    Serial.println("Failed to begin keypad");
  }

  hook_light.begin();

  tones.begin();

  AudioSequences::init(&tones);

  ui_effects.startup_sequence();
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

enum Outcomes : uint8_t {
  OUTCOME_RING,
  OUTCOME_BUSY,
  OUTCOME_REORDER,
  OUTCOME_ERROR
};

Outcomes outcome;

Outcomes determine_outcome(const char * pressed_digits, int8_t num_digits){
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

void start_outcome(Outcomes outcome){
  if(call_type == CALL_INTL){
    switch(outcome){
      case OUTCOME_RING:
        AudioSequences::uk_ring_sequence.start(8);
        break;
      case OUTCOME_BUSY:
        AudioSequences::uk_busy_sequence.start(12);
        break;
      case OUTCOME_REORDER:
        AudioSequences::uk_reorder_sequence.start(24);
        break;
      case OUTCOME_ERROR:
        AudioSequences::uk_error_sequence.start(10);
        break;
    }
  } else {
    switch(outcome){
      case OUTCOME_RING:
        AudioSequences::ring_sequence.start(6);
        break;
      case OUTCOME_BUSY:
        AudioSequences::busy_sequence.start(12);
        break;
      case OUTCOME_REORDER:
        AudioSequences::reorder_sequence.start(24);
        break;
      case OUTCOME_ERROR:
        AudioSequences::error_sequence.start(1);
        break;
    }
  }
}

bool step_outcome(Outcomes outcome){
  bool keep_going;

  if(call_type == CALL_INTL){
    switch(outcome){
      case OUTCOME_RING:
        keep_going = AudioSequences::uk_ring_sequence.step();
        break;
      case OUTCOME_BUSY:
        keep_going = AudioSequences::uk_busy_sequence.step();
        break;
      case OUTCOME_REORDER:
        keep_going = AudioSequences::uk_reorder_sequence.step();
        break;
      case OUTCOME_ERROR:
        keep_going = AudioSequences::uk_error_sequence.step();
        break;
      default:
        keep_going = false;
    }
  } else {
    switch(outcome){
      case OUTCOME_RING:
        keep_going = AudioSequences::ring_sequence.step();
        break;
      case OUTCOME_BUSY:
        keep_going = AudioSequences::busy_sequence.step();
        break;
      case OUTCOME_REORDER:
        keep_going = AudioSequences::reorder_sequence.step();
        break;
      case OUTCOME_ERROR:
        keep_going = AudioSequences::error_sequence.step();
        break;
      default:
        keep_going = false;
    }
  }
  return keep_going;
}

void action_dial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    tones.dial_key(key);
  }
}

void action_undial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    tones.sound_off();
  }
}

void action_dtmf(int8_t key, char ch){
  tones.dial_key(key);
}

void action_undtmf(int8_t key, char ch){
  tones.sound_off();
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

enum TopLevelStates : uint8_t {
  TOP_LEVEL_STATE_WAITING,
  TOP_LEVEL_STATE_INITIATE_CALL,
  TOP_LEVEL_STATE_CALL_START,
  TOP_LEVEL_STATE_CALL_IN_PROGRESS,
  TOP_LEVEL_STATE_ROUTING_START,
  TOP_LEVEL_STATE_ROUTING_IN_PROGRESS,
  TOP_LEVEL_STATE_COMMAND_B,
  TOP_LEVEL_STATE_COMMAND_C,
  TOP_LEVEL_STATE_COMMAND_D
};

TopLevelStates mode = TOP_LEVEL_STATE_WAITING;

void loop()
{
  char ch;
  switch(mode){
    case TOP_LEVEL_STATE_WAITING:
      if('\0' != (ch = keypad_handler.wait_for_char("ABCD", 1000, KeypadHandler::STATE_CONTINUED_KEY_PRESS, NULL, NULL))){
        switch(ch){
          case 'A':
            mode = TOP_LEVEL_STATE_INITIATE_CALL;
            break;
          case 'B':
            mode = TOP_LEVEL_STATE_COMMAND_B;
            break;
          case 'C':
            mode = TOP_LEVEL_STATE_COMMAND_C;
            break;
          case 'D':
            mode = TOP_LEVEL_STATE_COMMAND_D;
            break;
        }
      }
      break;
    case TOP_LEVEL_STATE_INITIATE_CALL:
      hook_light.on();
      tones.dial_tone();
      mode = TOP_LEVEL_STATE_CALL_START;
      // edge triggered key may still be pressed
      while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
      break;

    case TOP_LEVEL_STATE_CALL_START:
      reset_call();
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          mode = TOP_LEVEL_STATE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          ui_effects.blocking_error_tone();
          delay(200);
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          mode = TOP_LEVEL_STATE_WAITING;
        } 
        else {
          add_digit(ch);
          determine_routing(ch);
          mode = TOP_LEVEL_STATE_CALL_IN_PROGRESS;
        }
      }
      break;

    case TOP_LEVEL_STATE_CALL_IN_PROGRESS:
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          mode = TOP_LEVEL_STATE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          ui_effects.blocking_error_tone();
          delay(200);
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          mode = TOP_LEVEL_STATE_WAITING;
        } else {
          add_digit(ch);
          if(num_digits >= digit_count){
            mode = TOP_LEVEL_STATE_ROUTING_START;
          }
        }
      }
      break;

    case TOP_LEVEL_STATE_ROUTING_START:
        start_outcome(determine_outcome(digits, num_digits));
        ui_effects.blocking_pre_routing_sound();
        mode = TOP_LEVEL_STATE_ROUTING_IN_PROGRESS; 
      break;

    case TOP_LEVEL_STATE_ROUTING_IN_PROGRESS:
      if(keypad_handler.keypad_pressed()){
        tones.sound_off();
        // edge triggered key may still be pressed
        while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
        hook_light.off();
        ui_effects.blocking_cancel_tone();
        mode = TOP_LEVEL_STATE_WAITING;
      }
      if(!step_outcome(outcome)){
        tones.sound_off();
        ui_effects.blocking_post_routing_sound();
        hook_light.off();
       mode = TOP_LEVEL_STATE_WAITING;
      }
      break;

    case TOP_LEVEL_STATE_COMMAND_B:
    case TOP_LEVEL_STATE_COMMAND_C:
    case TOP_LEVEL_STATE_COMMAND_D:
      delay(200);
      tones.disconnect_tone();
      mode = TOP_LEVEL_STATE_WAITING;
      break;
  }
}
