#ifndef SIMCALL_HOOK_LIGHT_H
#define SIMCALL_HOOK_LIGHT_H

#include <Arduino.h>

class HookLight{
public:
    HookLight(uint8_t pin);

    void on();

    void off();

    void wink();

private:
    uint8_t _pin;
};


#endif
