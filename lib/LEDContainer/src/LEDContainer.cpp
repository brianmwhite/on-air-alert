#include "LEDContainer.h"
#include <Arduino.h>

LEDContainer::LEDContainer()
{
  LED_SIGNAL = LOW;
  READY_STATE = OFF;
  BLINK_DELAY_MILLISECONDS = 1000;
  NEXT_TIME_TO_BLINK = 0;
}

void LEDContainer::init(int pin)
{
  init(pin, BLINK_DELAY_MILLISECONDS);
}

void LEDContainer::init(int pin, int delay_in_milliseconds)
{
  PIN = pin;
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  BLINK_DELAY_MILLISECONDS = delay_in_milliseconds;
  updateLED();
}

void LEDContainer::setStatus(LEDState state)
{
  READY_STATE = state;
  updateLED();
}

void LEDContainer::setStatus(bool isOk)
{
  if (isOk)
  {
    setStatus(ON);
  }
  else
  {
    setStatus(OFF);
  }
}

void LEDContainer::blinkLED()
{
  int TIME_NOW = millis();
  if (TIME_NOW >= NEXT_TIME_TO_BLINK)
  {
    if (LED_SIGNAL == HIGH)
    {
      LED_SIGNAL = LOW;
    }
    else
    {
      LED_SIGNAL = HIGH;
    }
    NEXT_TIME_TO_BLINK = TIME_NOW + BLINK_DELAY_MILLISECONDS;
  }
}

void LEDContainer::updateLED()
{
  if (READY_STATE == ON)
  {
    LED_SIGNAL = HIGH;
  }
  else if (READY_STATE == BLINK)
  {
    blinkLED();
  }
  else
  {
    LED_SIGNAL = LOW;
  }
  digitalWrite(PIN, LED_SIGNAL);
}