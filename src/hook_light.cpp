#include <Arduino.h>
#include "hook_light.h"

HookLight::HookLight(uint8_t pin) : _pin(pin) {}

void HookLight::begin()
{
  pinMode(_pin, OUTPUT);
  off();
}

void HookLight::on()
{
  digitalWrite(_pin, LOW);
}

void HookLight::off()
{
  digitalWrite(_pin, HIGH);
}

void HookLight::wink()
{
  digitalWrite(_pin, digitalRead(_pin) == HIGH ? LOW : HIGH);
}

