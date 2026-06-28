#include "Wire.h"
#include <SPI.h>
#include "I2CKeyPad.h"
#include <MD_AD9833.h>
#include "types.h"
#include <random_seed.h>

// +----------+--------------------+-------------------------+
// | Keypad   | MF Signaling Tone  | Frequency Pair (Hz)     |
// +----------+--------------------+-------------------------+
// |    1     | Digit 1            |  700 Hz  +  900 Hz      |
// |    2     | Digit 2            |  700 Hz  + 1100 Hz      |
// |    3     | Digit 3            |  900 Hz  + 1100 Hz      |
// |    4     | Digit 4            |  700 Hz  + 1300 Hz      |
// |    5     | Digit 5            |  900 Hz  + 1300 Hz      |
// |    6     | Digit 6            | 1100 Hz  + 1300 Hz      |
// |    7     | Digit 7            |  700 Hz  + 1500 Hz      |
// |    8     | Digit 8            |  900 Hz  + 1500 Hz      |
// |    9     | Digit 9            | 1100 Hz  + 1500 Hz      |
// |    0     | Digit 0 (or 10)    | 1300 Hz  + 1500 Hz      |
// +----------+--------------------+-------------------------+
// |    *     | KP1 (Domestic)     | 1100 Hz  + 1700 Hz      |
// |  [Custom]| KP2 (International)| 1300 Hz  + 1700 Hz      |
// |    #     | ST (Start)         | 1500 Hz  + 1700 Hz      |
// +----------+--------------------+-------------------------+
// |    D     | SF Control Tone    | 2600 Hz  (Single Pure)  |
// +----------+--------------------+-------------------------+


#define RANDOM_SEED_PIN A1            // floating pin for seeding the RNG

static RandomSeed<RANDOM_SEED_PIN> randomizer;

const uint8_t KEYPAD_ADDRESS = 0x20;
I2CKeyPad keyPad(KEYPAD_ADDRESS);
char keymap[19] = "D#0*C987B654A321INF";  //  N = NoKey, F = Fail

// Pins for SPI comm with the AD9833 IC
const uint8_t PIN_DATA = 11;  ///< SPI Data pin number
const uint8_t PIN_CLK = 13;  	///< SPI Clock pin number
const uint8_t PIN_FSYNC1 = A2; //8; ///< SPI Load pin number (FSYNC in AD9833 usage)
const uint8_t PIN_FSYNC2 = A3; //7;  ///< SPI Load pin number (FSYNC in AD9833 usage)

const uint8_t HOOK_LIGHT_PIN = A1;

MD_AD9833	AD1(PIN_DATA, PIN_CLK, PIN_FSYNC1); // Arbitrary SPI pins
MD_AD9833	AD2(PIN_DATA, PIN_CLK, PIN_FSYNC2); // Arbitrary SPI pins

#define MIN_KEYPRESS_TIME 20

KeypadHandler keypad_handler(&keyPad, MIN_KEYPRESS_TIME);

// #define SILENT_FREQ 1000000
#define SILENT_FREQ 50000

class DtmfFreqencies
{
public:
  // keymap specifies the characters associated with the 4x4 keypad from the bottom right key to the top left key (down->up, right->left)
  // this is the same order the I2CKeyPad expects
  DtmfFreqencies(){
    _rows[0] = 697;
    _rows[1] = 770;
    _rows[2] = 852;
    _rows[3] = 941;
    _cols[0] = 1209;
    _cols[1] = 1336;
    _cols[2] = 1477;
    _cols[3] = 1633;
  }

  int row_freq_from_key(char key){
    char row = (15 - key) / 4;
    return _rows[row];
  }

  int col_freq_from_key(char key){
    char col = (15 - key) % 4;
    return _cols[col];
  }

private:
  int _rows[4];
  int _cols[4];
};

DtmfFreqencies dtmf;

#define DIAL_TONE_A 350.0
#define DIAL_TONE_B 440.0

bool begin_keypad(I2CKeyPad& keypad, char * keymap, const uint8_t address = 0x20){
  if(!keyPad.begin()){
    return false;
  }

  keyPad.loadKeyMap(keymap);
  return true;
}

void setup() {
  randomizer.randomize();
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);

  if(!begin_keypad(keyPad, keymap)){
    Serial.println("Failed to begin keypad");
  }

  AD1.begin();
  AD2.begin();
  AD1.setFrequency(0, SILENT_FREQ);
  AD1.setMode(MD_AD9833::MODE_SINE);

  AD2.setFrequency(0, SILENT_FREQ);
  AD2.setMode(MD_AD9833::MODE_SINE);

  pinMode(HOOK_LIGHT_PIN, OUTPUT);
  digitalWrite(HOOK_LIGHT_PIN, HIGH);
}

void hook_light_on(){
  digitalWrite(HOOK_LIGHT_PIN, LOW);
}

void hook_light_off(){
  digitalWrite(HOOK_LIGHT_PIN, HIGH);
}

void hook_link_wink(){
  digitalWrite(HOOK_LIGHT_PIN, digitalRead(HOOK_LIGHT_PIN) == HIGH ? LOW : HIGH);
}

void sound_off(){
  AD1.setFrequency(0, SILENT_FREQ);
  AD2.setFrequency(0, SILENT_FREQ);
}

void busy_on(){
  AD1.setFrequency(0, 480.0);
  AD2.setFrequency(0, 620.0);
}

void uk_busy_on(){
  AD1.setFrequency(0, 400.0);
}

void ring_on(){
  AD1.setFrequency(0, 480.0);
  AD2.setFrequency(0, 440.0);
}

void uk_ring_on(){
  AD1.setFrequency(0, 400.0);
  AD2.setFrequency(0, 450.0);
}

void error_tone1_on(){
  AD1.setFrequency(0, 913.8);
}

void error_tone2_on(){
  AD1.setFrequency(0, 1428.5);
}

void error_tone3_on(){
  AD1.setFrequency(0, 1776.7);
}

void cancel_tone_on(){
  AD1.setFrequency(0, 941);
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

void dual_tone(int freq1, int freq2, int times, int inter_delay, int final_delay = -1){
  final_delay = (final_delay == -1 ? inter_delay : final_delay);
  freq2 = (freq2 == 0 ? SILENT_FREQ : freq2);
  for(int i = 0; i < times; i++){
    AD1.setFrequency(0, freq1);
    AD2.setFrequency(0, freq2);
    delay(inter_delay);
    AD1.setFrequency(0, SILENT_FREQ);
    AD2.setFrequency(0, SILENT_FREQ);
    delay(i == times-1 ? final_delay : inter_delay);
  }  
}

void pop(){
  hook_link_wink();
  dual_tone(200, 200, 1, 7, 0);
  hook_link_wink();
}

void click(){
  hook_link_wink();
  dual_tone(600, 600, 1, 3, 0);
  hook_link_wink();
}

void dial_tone(){
  AD1.setFrequency(0, 350.0);
  AD2.setFrequency(0, 440.0);
}

// void confirmation_tone(){
//   dual_tone(350, 440, 3, 100);
// }

void disconnect_tone(){
  dual_tone(2600, 0, 1, 200);
}

void cancel_tone(){
  cancel_sequence.start(1);
  while(cancel_sequence.step());
}

void dial_key(int key){
  // AD1.setFrequency(0, rows[mapr[key]]);
  // AD2.setFrequency(0, cols[mapc[key]]);

  AD1.setFrequency(0, dtmf.row_freq_from_key(key));
  AD2.setFrequency(0, dtmf.col_freq_from_key(key));
}

void error_tone(){
  delay(200);
  error_sequence.start(1);
  while(error_sequence.step());
}

#define MAX_DIGITS 16
char digits[MAX_DIGITS+1];
char num_digits = 0;

#define CALL_NONE 0
// 867-5209
#define CALL_LOCAL 1
#define LOCAL_COUNT 7
// 1-800-555-1212
#define CALL_LONG 2
#define LONG_PREFIX '1'
#define LONG_COUNT 11
// 011-44-8302-1212
#define CALL_INTL 3
#define INTL_PREFIX '0'
#define INTL_COUNT 13

int digit_count = 0;
int call_type = CALL_NONE;

#define OUTCOME_RING 0
#define OUTCOME_BUSY 1
#define OUTCOME_REORDER 2
#define OUTCOME_ERROR 3

int outcome;

int determine_outcome(char * digits, int num_digits){
  switch(digits[num_digits-1]){
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
    }
  }
  return keep_going;
}

void pre_routing_sound(){
  // random delays, clicks, pops, other routing effects here
  delay(random(500, 1000));
  // pop();
  click();
  delay(random(500, 1000));
  pop();
  // delay(random(250, 500));
  delay(300);
}

void post_routing_sound(){
  delay(500);
  pop();
}

void action_dial(char key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    dial_key(key);
  }
}

void action_undial(char key, char ch){
  if(KeypadHandler::char_in_chars(ch, "0123456789*#")){
    sound_off();
  }
}

void action_dtmf(char key, char ch){
  dial_key(key);
}

void action_undtmf(char key, char ch){
  sound_off();
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

#define MODE_WAITING 0
#define MODE_INITIATE_CALL 1
#define MODE_CALL_START 2
#define MODE_CALL_IN_PROGRESS 3
#define MODE_ROUTING_START 4
#define MODE_ROUTING_IN_PROGRESS 5
#define MODE_COMMAND_B 20
#define MODE_COMMAND_C 30
#define MODE_COMMAND_D 40
int mode = MODE_WAITING;

void loop()
{
  char ch;
  switch(mode){
    case MODE_WAITING:
      if(NULL != (ch = keypad_handler.wait_for_char("ABCD", 1000, KeypadHandler::STATE_CONTINUED_KEY_PRESS, NULL, NULL))){
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
      hook_light_on();
      dial_tone();
      mode = MODE_CALL_START;
      // edge triggered key may still be pressed
      while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
      break;

    case MODE_CALL_START:
      reset_call();
      ch = keypad_handler.wait_for_char("0123456789*#A", 1000, KeypadHandler::STATE_IDLE, action_dial, action_undial);
      if(ch != NULL){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light_off();
          cancel_tone();
          mode = MODE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          error_tone();
          delay(200);
          hook_light_off();
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
      if(ch != NULL){
        if(KeypadHandler::char_in_chars(ch, "A")){
          hook_light_off();
          cancel_tone();
          mode = MODE_WAITING;
          break;
        } else if(KeypadHandler::char_in_chars(ch, "*#")){
          error_tone();
          delay(200);
          hook_light_off();
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
        sound_off();
        // edge triggered key may still be pressed
        while(!keypad_handler.keypad_state_wait(KeypadHandler::STATE_IDLE, action_dial, action_undial));
        hook_light_off();
        cancel_tone();
        mode = MODE_WAITING;
      }
      if(!step_outcome(outcome)){
        sound_off();
        post_routing_sound();
        hook_light_off();
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
