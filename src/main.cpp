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
Tones tones(&AD1, &AD2, SILENT_FREQ, &keypad_handler);

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

enum RoutingTypes : uint8_t {
  ROUTING_NONE,
  ROUTING_LOCAL,
  ROUTING_LONG,
  ROUTING_OPER_OR_INTL,
  ROUTING_INTL,
  ROUTING_OPER,
  ROUTING_ERROR,
};

const uint8_t MAX_DIGITS = 22;
char digits[MAX_DIGITS+1];
int8_t num_digits = 0;

// const uint8_t ROUTING_NONE = 0;
// 867-5209
// const uint8_t ROUTING_LOCAL = 1;
const uint8_t LOCAL_COUNT = 7;
// 1-800-555-1212
// const uint8_t ROUTING_LONG = 2;
const char LONG_PREFIX = '1';
const uint8_t LONG_COUNT = 11;
// 011-44-8302-1212
// const uint8_t ROUTING_INTL = 3;
const char INTL_PREFIX = '0';
const uint8_t INTL_COUNT = 13;

const uint8_t OPER_COUNT = 1;
const uint8_t ERROR_COUNT = 0;

int digit_count = 0;
RoutingTypes routing_type = ROUTING_NONE;

void reset_call(){
  num_digits = 0;
  digit_count = 0;
  routing_type = ROUTING_NONE;
}

void add_digit(char ch){
  if(num_digits < MAX_DIGITS){
    digits[num_digits++] = ch;
    digits[num_digits] = '\0'; // stringify
  }
}

// used to prefix the digit sequence with KP1
void insert_digit(char ch, bool skip_if_present=true){
  Serial.println(digits);
  if(skip_if_present == false || digits[0] != ch){
    if(num_digits > 0){
      for(int i = num_digits + 1; i > 0; i--){
        digits[i] = digits[i-1];
      }
      digits[0] = ch;
    }  
  }
  Serial.println(digits);
}

bool determine_routing(){
  bool error = false;
  char digit1;
  char digit2;
  char digit3;

  if(keypad_handler.char_in_chars('*', digits) || keypad_handler.char_in_chars('#', digits)){
    routing_type = ROUTING_ERROR;
    digit_count = ERROR_COUNT;
    return false;
  }

  switch(num_digits){
    case 0:
      // nothing dialed yet
      routing_type = ROUTING_NONE;
      break;

    case 1:
      digit1 = digits[0];

      switch(digit1){
        case '0':
          // first dialed digit is a 0, either calling operator or international
          routing_type = ROUTING_OPER_OR_INTL;
          digit_count = INTL_COUNT;
          break;
        case '1':
          // first dialed digit is a 1, calling long distance
          routing_type = ROUTING_LONG;
          digit_count = LONG_COUNT;
          break;
        default:
          // first dialed digit is 2-9, calling local 
          routing_type = ROUTING_LOCAL;
          digit_count = LOCAL_COUNT;
          break;
      }
      break;

    case 2:
      digit1 = digits[0];
      digit2 = digits[1];

      switch(digit2){
        case '0':
          if(digit1 == '0'){
            // second dialed digit is 0 when first was also 0, calling (long distance) operator (without a time out)
            routing_type = ROUTING_OPER;
            digit_count = OPER_COUNT;
          } else if(digit1 == '1'){
            // second dialed digit is a 0 when first was a 1, error
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else {
            // second dialed digit is 0 when first was a 2-9, calling local new area code-like exchange prefix
            routing_type = ROUTING_LOCAL;
            digit_count = LOCAL_COUNT; // account for the the leading zero
            break;
          }
          break;
        case '1':
          if(digit1 == '0'){
            // second dialed digit is 1 when first was 0, calling international or operator (assist local or long time out)
            routing_type = ROUTING_OPER_OR_INTL;
            digit_count = INTL_COUNT;

          } else if(digit1 == '1'){
            // second dialed digit is a 1 when first was a 1, error
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else {
            // second dialed digit is 1 when first was a 2-9, calling local new area code-like exchange prefix
            routing_type = ROUTING_LOCAL;
            digit_count = LOCAL_COUNT; // account for the the leading zero
            break;
          }
          break;
      }
      break;

    case 3:
      digit1 = digits[0];
      digit2 = digits[1];
      digit3 = digits[2];
  
      switch(digit3){
        case '0':
          if(digit1 == '0' && digit2 == '0'){
            // third dialed digit is 0 when first two were also 0, error (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
          } else if(digit1 == '0' && digit2 == '1'){
            // third dialed digit is a 0 when first was a 0 and second was a 1, error
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else if(digit1 == '1' && digit2 == '0'){
            // third dialed digit is a 0 when first was a 1 and second was a 0, error  (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else if(digit1 == '1' && digit2 == '1'){
            // third dialed digit is a 0 when first was a 1 and second was a 1, error  (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } 
          else {
            // third dialed digit is 0 when first and second was a 2-9, calling local new area code-like exchange prefix (if an exchange like 790 is allowed)
            routing_type = ROUTING_LOCAL;
            digit_count = LOCAL_COUNT; // account for the the leading zero
            break;
          }
        case '1':
          if(digit1 == '0' && digit2 == '0'){
            // third dialed digit is 1 when first was 0 and second was 0, error (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else if(digit1 == '0' && digit2 == '1'){
            // third dialed digit is 1 when the first was 0 and the second was 1, international
            routing_type = ROUTING_INTL;
            digit_count = INTL_COUNT;
          } else if(digit1 == '1' && digit2 == '0'){
            // third dialed digit is 1 when first was 1 and second was 0, error (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else if(digit1 == '1' && digit2 == '1'){
            // third dialed digit is 1 when first was 1 and second was 1, error (this case should be caught and handled before the third digit)
            routing_type = ROUTING_ERROR;
            digit_count = ERROR_COUNT;
            error = true;
            break;
          } else {
            // third dialed digit is 1 when first, and second was a 2-9, calling local new area code-like exchange prefix
            routing_type = ROUTING_LOCAL;
            digit_count = LOCAL_COUNT; // account for the the leading zero
            break;
          }
          break;
      }
      break;
  }

  return !error;
}

// operator routing is determined after the digits are cached
// 1 44 444 44444
// 800 555 1212
void determine_oprouting(){
  // for international calls assume 11 more more digits
  // for regular calls assume less (domestic long distance won't include the intitial "1")
  if(num_digits >= 11)
  {
    routing_type = ROUTING_INTL;
  } else if(num_digits >= 10) {
    routing_type = ROUTING_LONG;
  } else {
    routing_type = ROUTING_LOCAL;
  }
}

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

  if(routing_type == ROUTING_INTL){
    if(r < 580){
      outcome = OUTCOME_RING;
    } else if(r < 830){
      outcome = OUTCOME_BUSY;
    } else if(r < 950){
      outcome = OUTCOME_REORDER;
    } else {
      outcome = OUTCOME_ERROR;
    }
  } else if(routing_type == ROUTING_LONG)
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
  if(routing_type == ROUTING_INTL){
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

  if(routing_type == ROUTING_INTL){
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

void action_opdial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#ACD")){
    tones.dial_opkey(key);
  }
}

void action_unopdial(int8_t key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#ACD")){
    tones.sound_off();
  }
}

enum TopLevelStates : uint8_t {
  TOP_LEVEL_STATE_WAITING,

  TOP_LEVEL_STATE_INITIATE_CALL,
  TOP_LEVEL_STATE_CALL_START,
  TOP_LEVEL_STATE_CALL_IN_PROGRESS,
  TOP_LEVEL_STATE_ROUTING_START,
  TOP_LEVEL_STATE_ROUTING_ERROR,
  TOP_LEVEL_STATE_ROUTING_IN_PROGRESS,
  
  TOP_LEVEL_STATE_INITIATE_OPCALL,
  TOP_LEVEL_STATE_OPCALL_START,
  TOP_LEVEL_STATE_OPCALL_IN_PROGRESS,
  TOP_LEVEL_STATE_OPROUTING_DISCONNECT,
  TOP_LEVEL_STATE_OPROUTING_WINK,
  TOP_LEVEL_STATE_OPROUTING_AUTODIAL,
  TOP_LEVEL_STATE_OPROUTING_START,
  TOP_LEVEL_STATE_OPROUTING_IN_PROGRESS,
  
  TOP_LEVEL_STATE_COMMAND_C,
  TOP_LEVEL_STATE_COMMAND_D
};

TopLevelStates top_level_state = TOP_LEVEL_STATE_WAITING;

void loop()
{
  char ch;
  switch(top_level_state){
    case TOP_LEVEL_STATE_WAITING:
      if('\0' != (ch = keypad_handler.wait_for_char("ABCD", 1000, KeypadHandler::STATE_CONTINUED_KEY_PRESS, nullptr, nullptr))){
        switch(ch){
          case 'A':
            top_level_state = TOP_LEVEL_STATE_INITIATE_CALL;
            break;
          case 'B':
            top_level_state = TOP_LEVEL_STATE_INITIATE_OPCALL;
            break;
          case 'C':
            top_level_state = TOP_LEVEL_STATE_COMMAND_C;
            break;
          case 'D':
            top_level_state = TOP_LEVEL_STATE_COMMAND_D;
            break;
        }
      }
      break;


    case TOP_LEVEL_STATE_INITIATE_CALL:
      hook_light.on();
      tones.dial_tone();
      top_level_state = TOP_LEVEL_STATE_CALL_START;
      // edge triggered key may still be pressed
      while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
      break;

    case TOP_LEVEL_STATE_CALL_START:
      reset_call();
      ch = keypad_handler.wait_for_char("0123456789*#AD", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          top_level_state = TOP_LEVEL_STATE_WAITING;
        } else if(KeypadHandler::char_in_chars(ch, "D")){
          tones.blocking_dial_sequence(digits);
          top_level_state = TOP_LEVEL_STATE_ROUTING_START;
        } 
        else {
          add_digit(ch);
          if(!determine_routing()){
            top_level_state = TOP_LEVEL_STATE_ROUTING_ERROR;
            break;
          }
          top_level_state = TOP_LEVEL_STATE_CALL_IN_PROGRESS;
        }
      }
      break;

    case TOP_LEVEL_STATE_ROUTING_ERROR:
      reset_call();
      ui_effects.blocking_error_tone();
      delay(200);
      hook_light.off();
      ui_effects.blocking_cancel_tone();
      top_level_state = TOP_LEVEL_STATE_WAITING;
      break;

    // TODO: the above and below states have nearly identical code to each other and the TOP_LEVEL_STATE_OPCALL_START/IN_PROGRESS states

    case TOP_LEVEL_STATE_CALL_IN_PROGRESS:
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          top_level_state = TOP_LEVEL_STATE_WAITING;
          break;
        } else {
          add_digit(ch);
          if(!determine_routing()){
            top_level_state = TOP_LEVEL_STATE_ROUTING_ERROR;
            break;
          }
          if(num_digits >= digit_count){
            top_level_state = TOP_LEVEL_STATE_ROUTING_START;
          }
        }
      }
      break;

    case TOP_LEVEL_STATE_ROUTING_START:
        start_outcome(determine_outcome(digits, num_digits));
        ui_effects.blocking_pre_routing_sound();
        top_level_state = TOP_LEVEL_STATE_ROUTING_IN_PROGRESS; 
      break;

    case TOP_LEVEL_STATE_ROUTING_IN_PROGRESS:
      // any pressed key halts routing
      if(keypad_handler.keypad_pressed()){
        tones.sound_off();
        // edge triggered key may still be pressed
        while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
        hook_light.off();
        ui_effects.blocking_cancel_tone();
        top_level_state = TOP_LEVEL_STATE_WAITING;
        break;
      }
      if(!step_outcome(outcome)){
        tones.sound_off();
        ui_effects.blocking_post_routing_sound();
        hook_light.off();
       top_level_state = TOP_LEVEL_STATE_WAITING;
      }
      break;


    case TOP_LEVEL_STATE_INITIATE_OPCALL:
      ui_effects.blocking_ready_tone();
      top_level_state = TOP_LEVEL_STATE_OPCALL_START;
      // edge triggered key may still be pressed
      while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_opdial, action_unopdial));
      break;

    case TOP_LEVEL_STATE_OPCALL_START:
      reset_call();
      ch = keypad_handler.wait_for_char(nullptr, 1000, KeypadHandler::STATE_IDLE, action_opdial, action_unopdial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "B")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          top_level_state = TOP_LEVEL_STATE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "D")){
          tones.blocking_dial_sequence(digits, true);
          determine_oprouting(); // this looks at number of digits
          // insert_digit('*');
          // add_digit('#');
          top_level_state = TOP_LEVEL_STATE_OPROUTING_DISCONNECT;
        } 
        else if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
          add_digit(ch);
          top_level_state = TOP_LEVEL_STATE_OPCALL_IN_PROGRESS;
        }
      }
      break;

    // TODO: the above and below states have nearly identical code to each other and the TOP_LEVEL_STATE_CALL_START/IN_PROGRESS states

    case TOP_LEVEL_STATE_OPCALL_IN_PROGRESS:
      ch = keypad_handler.wait_for_char(nullptr, 1000, KeypadHandler::STATE_IDLE, action_opdial, action_unopdial);
      if(ch != '\0'){
        if(KeypadHandler::char_in_chars(ch, "B")){
          hook_light.off();
          ui_effects.blocking_cancel_tone();
          top_level_state = TOP_LEVEL_STATE_WAITING;
          break;
        } 
        else if(KeypadHandler::char_in_chars(ch, "#C")){
          // load a ST tone into the digit buffer so it plays later on autodial
          determine_oprouting(); // this looks at number of digits
          insert_digit('*');
          add_digit('#');
          top_level_state = TOP_LEVEL_STATE_OPROUTING_DISCONNECT;
        } 
        else if(KeypadHandler::char_in_chars(ch, "0123456789")){
          add_digit(ch);
        }
      }
      break;

    case TOP_LEVEL_STATE_OPROUTING_DISCONNECT:
      ui_effects.blocking_disconnect();
      top_level_state = TOP_LEVEL_STATE_OPROUTING_WINK;
      break;

    case TOP_LEVEL_STATE_OPROUTING_WINK:
      ui_effects.blocking_wink();
      top_level_state = TOP_LEVEL_STATE_OPROUTING_AUTODIAL;
      break;

    case TOP_LEVEL_STATE_OPROUTING_AUTODIAL:
      tones.blocking_dial_sequence(digits, true, 55, 50);
      top_level_state = TOP_LEVEL_STATE_OPROUTING_START;
      break;

    case TOP_LEVEL_STATE_OPROUTING_START:
      hook_light.on();
      start_outcome(determine_outcome(digits, num_digits));
      ui_effects.blocking_pre_routing_sound();
      top_level_state = TOP_LEVEL_STATE_OPROUTING_IN_PROGRESS; 
      break;

      case TOP_LEVEL_STATE_OPROUTING_IN_PROGRESS:
      // any pressed key halts routing
      if(keypad_handler.keypad_pressed()){
        tones.sound_off();
        // edge triggered key may still be pressed
        while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_opdial, action_unopdial));
        hook_light.off();
        ui_effects.blocking_cancel_tone();
        top_level_state = TOP_LEVEL_STATE_WAITING;
        break;
      }
      if(!step_outcome(outcome)){
        tones.sound_off();
        ui_effects.blocking_post_routing_sound();
        hook_light.off();
       top_level_state = TOP_LEVEL_STATE_WAITING;
      }
      break;

    case TOP_LEVEL_STATE_COMMAND_C:
    case TOP_LEVEL_STATE_COMMAND_D:
      delay(200);
      tones.disconnect_tone();
      top_level_state = TOP_LEVEL_STATE_WAITING;
      break;
  }
}
