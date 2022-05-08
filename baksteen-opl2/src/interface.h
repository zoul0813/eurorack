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

  if (encoderSwitch.changed() && encoderSwitch.read())
  {
    select_menu++;
    if (select_menu > 5)
    {
      select_menu = 0;
    }

#if DEBUG == 1
    switch (select_menu)
    {
    case 0:
      Serial.println("Attack");
      break;
    case 1:
      Serial.println("Decay");
      break;
    case 2:
      Serial.println("Sustain");
      break;
    case 3:
      Serial.println("Release");
      break;
    case 4:
      Serial.println("Tremelo");
      break;
    case 5:
      Serial.println("Vibrato");
      break;
    }
#endif
  }

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