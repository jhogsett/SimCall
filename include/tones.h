#ifndef SIMCALL_TONES_H
#define SIMCALL_TONES_H

#include <Arduino.h>
#include <MD_AD9833.h>
#include "dtmf.h"
#include "r1mf.h"
#include "keypad_handler.h"

class Tones
{
public:
    Tones(MD_AD9833 * pDevice1, MD_AD9833 * pDevice2, float silent_freq, KeypadHandler * _pkeypad_handler);

    void begin();

    void sound_off();
    void busy_on();
    void uk_busy_on();
    void ring_on();
    void uk_ring_on();
    void error_tone1_on();
    void error_tone2_on();
    void error_tone3_on();
    void ready_tone_on();
    void cancel_tone_on();
    void disconnect_tone_on();
    void dual_tone(int freq1, int freq2, int times, int inter_delay, int final_delay = -1);
    void dial_tone();
    void confirmation_tone();
    void disconnect_tone();
    void dial_key(uint8_t key);
    void dial_opkey(uint8_t key);

    static constexpr int DEFAULT_REDIAL_DIGIT_TIME = 100;
    static constexpr int DEFAULT_REDIAL_INTERDIGIT_TIME = 100;

    void blocking_dial_sequence(const char * digits, bool use_opkeys=false, int digit_time=DEFAULT_REDIAL_DIGIT_TIME, int interdigit_time=DEFAULT_REDIAL_INTERDIGIT_TIME);

private:
    MD_AD9833 * _pDevice1;
    MD_AD9833 * _pDevice2;
    float _silent_freq;
    KeypadHandler * _pkeypad_handler;
};

#endif
