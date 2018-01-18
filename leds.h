#ifndef COFFEE_LEDS_H
#define COFFEE_LEDS_H

#include "time.h"

#define LED_OFF 0
#define LED_FLASH 1
#define LED_ON 2

#define FLASH_ON_DURATION_S 0.5
#define FLASH_DURATION_S 1.0

struct LEDInfo {
  int* pins;
  int* states;
  int nLeds;
  time_t pulseStarted;
} ledInfo;

void ledsInit() {
  ledInfo.pins = NULL;
  ledInfo.states = NULL;
  ledInfo.nLeds = 0;
  time(&ledInfo.pulseStarted);
}

void ledsSetState(int pin, int state) {
  int i;
  int* pinsCpy;
  int* statesCpy;
  for(i = 0; i < ledInfo.nLeds; i++) {
    if(ledInfo.pins[i] == pin) {
      break;
    }
  }
  if(i == ledInfo.nLeds) {
    pinsCpy = new int[ledInfo.nLeds + 1];
    statesCpy = new int[ledInfo.nLeds + 1];
    memcpy(pinsCpy, ledInfo.pins, sizeof(int) * ledInfo.nLeds);
    memcpy(statesCpy, ledInfo.states, sizeof(int) * ledInfo.nLeds);
    delete[] ledInfo.pins;
    delete[] ledInfo.states;
    ledInfo.pins = pinsCpy;
    ledInfo.states = statesCpy;
    ledInfo.nLeds = ledInfo.nLeds + 1;
    ledInfo.pins[i] = pin;
    pinMode(pin, OUTPUT);
  }
  ledInfo.states[i] = state;
}

void ledsUpdate() {
  time_t now;
  int i, flashState;
  double delta;
  time(&now);
  delta = difftime(now, ledInfo.pulseStarted);
  while(delta >= FLASH_DURATION_S) {
    ledInfo.pulseStarted += (FLASH_DURATION_S * 1000);
    delta -= FLASH_DURATION_S;
  }
  //Serial.write(delta);
  if(delta > FLASH_ON_DURATION_S) {
    flashState = LOW;
  } else {
    flashState = HIGH;
  }
  for(i = 0; i < ledInfo.nLeds; i++) {
    switch(ledInfo.states[i]) {
      case LED_OFF: {
        digitalWrite(ledInfo.pins[i], LOW);
        break;
      }
      case LED_FLASH: {
        digitalWrite(ledInfo.pins[i], flashState);
        break;
      }
      case LED_ON: {
        digitalWrite(ledInfo.pins[i], HIGH);
        break;
      }
    }
  }
}

#endif
