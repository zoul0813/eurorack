#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "opl.h"
#include "shift.h"
#include "pixels.h"

#ifndef __INTERFACE_H
#define __INTERFACE_H

void interfaceInit();
void encoderISR();
void updateEncoder();
void interfaceProcess(uint8_t gates);

void interfaceInit() {
  encoderSwitch.attach(ENCODER_SW, INPUT_PULLUP);
  encoderSwitch.interval(5);

  encoder = new RotaryEncoder(ENCODER_1, ENCODER_2, ENCODER_LATCH);
  attachInterrupt(digitalPinToInterrupt(ENCODER_1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_2), encoderISR, CHANGE);
}

void interfaceProcess(uint8_t gates) {
  bool presetPrev = !bitRead(gates, CV_PREV);
  bool lastPresetPrev = bitRead(last_inputs, CV_PREV);

  bool presetNext = bitRead(gates, CV_NEXT);
  bool lastPresetNext = bitRead(last_inputs, CV_NEXT);

  updateEncoder();

  if (presetNext && !lastPresetNext) 
    nextInstrument();

  if (presetPrev && !lastPresetPrev)
    prevInstrument();

  if (encoderSwitch.changed() && encoderSwitch.read())
  {
    // 0 = Poly, 1 = Rhythm, 2 = Chord
    currentMode++;
    if (currentMode > 2)
    {
      currentMode = 0;
    }
  }
  updatePixels(gates);
}

void encoderISR()
{
  encoder->tick(); // just call tick() to check the state.
}

void updateEncoder()
{
  encoderSwitch.update();

  oldPosition = newPosition;
  newPosition = encoder->getPosition();
  RotaryEncoder::Direction encoderDirection = encoder->getDirection();

  if (encoderDirection == RotaryEncoder::Direction::COUNTERCLOCKWISE)
  { // turn left
    oldPosition = newPosition;
#if DEBUG == 1
    Serial.print("Encoder: Left, Menu: ");
    Serial.println(select_menu);
#endif
    updateADSR(-1);
  }
  else if (encoderDirection == RotaryEncoder::Direction::CLOCKWISE)
  { // turn right
    oldPosition = newPosition;
#if DEBUG == 1
    Serial.print("Encoder: Right, Menu: ");
    Serial.println(select_menu);
#endif
    updateADSR(1);
  }
  else
  {
    // Serial.print("Encoder: None, Menu: ");
  }
}


#endif