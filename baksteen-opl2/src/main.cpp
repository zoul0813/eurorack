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
#include "globals.h"
#include "opl.h"
#include "shift.h"
#include "interface.h"

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
  interfaceInit();
}

void loop()
{
  byte inputs = getShift();

  if(inputs != last_inputs) {
    // shift off the next/prev buttons, we only have 6 gates
    // TODO: reverse this so the gates are first 6 bits, remove this shift?
    oplProcess(inputs >> 2);
  }
  
  interfaceProcess(inputs);

  last_inputs = inputs;
}