#ifndef SIMCALL_TONES_H
#define SIMCALL_TONES_H

#include <Arduino.h>
#include <MD_AD9833.h>
#include "dtmf.h"

class Tones
{
public:
    Tones(MD_AD9833 * pDevice1, MD_AD9833 * pDevice2, float silent_freq);

    void begin();

    void sound_off();
    void busy_on();
    void uk_busy_on();
    void ring_on();
    void uk_ring_on();
    void error_tone1_on();
    void error_tone2_on();
    void error_tone3_on();
    void cancel_tone_on();
    void dual_tone(int freq1, int freq2, int times, int inter_delay, int final_delay = -1);
    void dial_tone();
    void confirmation_tone();
    void disconnect_tone();
    void dial_key(int key);

private:
    MD_AD9833 * _pDevice1;
    MD_AD9833 * _pDevice2;
    float _silent_freq;
    static Dtmf _dtmf;
};

#endif
