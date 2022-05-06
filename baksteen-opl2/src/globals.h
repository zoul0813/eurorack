#include <Arduino.h>
#include <Bounce2.h>
#include <RotaryEncoder.h>
#include "config.h"

#ifndef __VARS_H
#define __VARS_H

uint8_t currentMode = MODE_POLY;
uint16_t voct = 0; // 0-1023
uint16_t cv1 = 0;
uint16_t cv2 = 0;
byte playNote[VOICES] = {0, 0, 0, 0, 0, 0};
bool playedNote[VOICES] = { false, false, false, false, false, false, };
bool last_gate[VOICES] = { false, false, false, false, false, false, };

byte last_inputs = B00000000;

float VOLTAGE_OFFSET = 0.01;
uint8_t OCTAVE_BASE = 2;
uint8_t channel = 0;
int8_t currentInstrument = 0;

uint8_t attack = 0;
uint8_t decay = 0;
uint8_t sustain = 0;
uint8_t release = 0;
bool tremelo = false;
uint8_t tremelo_depth = 0;
bool vibrato = false;
uint8_t vibrato_depth = 0;

// Bounce presetNext = Bounce();
// Bounce presetPrev = Bounce();
Bounce encoderSwitch = Bounce();
RotaryEncoder *encoder = nullptr;

int oldPosition = -999;
int newPosition = -999;
int i = 0;
int8_t select_menu = 0; //
#endif