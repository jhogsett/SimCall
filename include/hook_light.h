#ifndef __HOOK_LIGHT_H__
#define __HOOK_LIGHT_H__

#include <Arduino.h>>

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
