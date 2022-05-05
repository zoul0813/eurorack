/**
 * This is a demonstration sketch for the OPL2 Audio Board. It demonstrates playing a little tune on 3 channels using
 * a piano from the MIDI instrument defenitions.
 *
 * OPL2 board is connected as follows:
 * Pin  8 - Reset   /7  .
 * Pin  9 - A0      /3  .
 * Pin 10 - Latch   /5  .
 * Pin 11 - Data    /4  .
 * Pin 13 - Shift   /6
 *
 * Refer to the wiki at https://github.com/DhrBaksteen/ArduinoOPL2/wiki/Connecting to learn how to connect your platform
 * of choice!
 *
 * Code by Maarten Janssen (maarten@cheerful.nl) 2016-04-13
 * Most recent version of the library can be found at my GitHub: https://github.com/DhrBaksteen/ArduinoOPL2
 */

#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <OPL2.h>
#include <Bounce2.h>
#include <RotaryEncoder.h>

#include "config.h"
#include "opl.h"
#include "globals.h"
#include "shift.h"

void encoderISR();
void updateEncoder();


void setup()
{
  oplInit();

#if DEBUG == 1
  Serial.begin(115200);
  Serial.print("OPL2 Eurorack Module - ");
  Serial.println(VERSION);
#endif

  // setup 74hc165 pins
  shiftInit();

  encoderSwitch.attach(ENCODER_SW, INPUT_PULLUP);
  encoderSwitch.interval(5);

  encoder = new RotaryEncoder(ENCODER_1, ENCODER_2, ENCODER_LATCH);
  attachInterrupt(digitalPinToInterrupt(ENCODER_1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_2), encoderISR, CHANGE);
}

void loop()
{
  byte gates = getShift();

  bool presetPrev = !bitRead(gates, CV_PREV);
  bool lastPresetPrev = bitRead(last_inputs, CV_PREV);

  bool presetNext = bitRead(gates, CV_NEXT);
  bool lastPresetNext = bitRead(last_inputs, CV_NEXT);

  voct = analogRead(CV_VOCT);
  mod = analogRead(CV_MOD);

#if DEBUG_VERBOSE == 1
  float voltage = voct * (5.0 / 1023.0);
#endif

  semitone = map(voct, 0, 1023, 0, 61);
  octave = semitone / 12;
  note = semitone - (octave * 12);
  octave += OCTAVE_BASE;

#if DEBUG_VERBOSE == 1
  if(isGate(0)) {
    Serial.print("V/OCT: ");

    Serial.print(semitone);
    Serial.print(", ");
    Serial.print(octave);
    Serial.print(", ");
    Serial.print(note);
    Serial.print(", ");
    Serial.print(channel);
    Serial.print(", ");
    Serial.print(mod);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.println(voct);
  }
#endif

  oplProcess(gates >> 2);

  last_mod = mod;
  last_inputs = gates;

  updateEncoder();

  if (presetNext && !lastPresetNext) 
    nextInstrument();

  if (presetPrev && !lastPresetPrev)
    prevInstrument();

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