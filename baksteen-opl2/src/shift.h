#include <Arduino.h>
#include "config.h"
#include "globals.h"

#ifndef __SHIFT_H
#define __SHIFT_H

void shiftInit();
byte getShift();
void setShift(byte b);

void shiftInit() {
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_CLKIN, OUTPUT);
  pinMode(SHIFT_LOAD, OUTPUT);
  pinMode(SHIFT_DATA, INPUT);
}


byte getShift()
{
  digitalWrite(SHIFT_LOAD, LOW);
  delayMicroseconds(5);
  digitalWrite(SHIFT_LOAD, HIGH);
  delayMicroseconds(5);

  digitalWrite(SHIFT_CLK, HIGH);
  digitalWrite(SHIFT_CLKIN, LOW);
  byte b = shiftIn(SHIFT_DATA, SHIFT_CLK, LSBFIRST);
  digitalWrite(SHIFT_CLKIN, HIGH);
#if DEBUG_VERBOSE == 1
  Serial.print("Shift: ");
  Serial.println(v, BIN);
#endif
  return b;
}

void setShift(byte b) {
  // shift the data out 
}

#endif