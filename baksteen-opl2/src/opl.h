#include <Arduino.h>
#include <OPL2.h>
#include <midi_instruments.h>
#include "config.h"
#include "globals.h"

#ifndef __OPL_H
#define __OPL_H
OPL2 opl2;
// uint16_t semitone = 0;
// uint8_t octave = 0;
// uint8_t note = 0;

// the +2 should be compiled out
#define isGate(bit) !(((gates) >> (bit)) & 0x01)
#define notGate(bit) (((gates) >> (bit)) & 0x01)

void oplInit();
void oplProcess(uint8_t gates);
bool doStopNote(uint8_t gates, uint8_t voice);
bool shouldPlayNote(uint8_t gates, uint8_t voice);
bool doPlayNote(uint8_t gates, uint8_t voice, uint16_t on);
void doRhythm(uint8_t gates);
void doPoly(uint8_t gates);
void doChord(uint8_t gates);
uint16_t voltToNote(uint16_t v);
void setInstrument(uint8_t index);
void nextInstrument();
void prevInstrument();
void updateInstrument();
void updateADSR(int8_t byRef);

void oplInit() {
  opl2.begin();
  currentInstrument = 0;

  switch(currentMode) {
    case MODE_POLY: {
      Instrument instrument = opl2.loadInstrument(INSTRUMENT_CRYSTAL); // Load a piano instrument.
      for(int i = 0; i < 9; i++) {
        opl2.setInstrument(i, instrument);                  // Assign the instrument to O
      }
    } break;
    case MODE_RHYTHM: {
      Instrument instrument = opl2.loadInstrument(INSTRUMENT_SHANNAI); // Load a piano instrument.
      opl2.setInstrument(1, instrument);                  // Assign the instrument to OPL2 channel 0.

      instrument = opl2.loadInstrument(INSTRUMENT_CRYSTAL); // Load a piano instrument.
      opl2.setInstrument(2, instrument);                  // Assign the instrument to OPL2 channel 0.

      instrument = opl2.loadInstrument(INSTRUMENT_TAIKO); // Load a piano instrument.
      opl2.setInstrument(3, instrument);                  // Assign the instrument to OPL2 channel 0.

      instrument = opl2.loadInstrument(INSTRUMENT_WOODBLOK); // Load a piano instrument.
      opl2.setInstrument(4, instrument);                  // Assign the instrument to OPL2 channel 0.

      instrument = opl2.loadInstrument(INSTRUMENT_STEELDRM); // Load a piano instrument.
      opl2.setInstrument(5, instrument);                  // Assign the instrument to OPL2 channel 0.
    } break;
    case MODE_CHORD: {
      Instrument instrument = opl2.loadInstrument(INSTRUMENT_VIOLIN); // Load a piano instrument.
      for(int i = 0; i < 9; i++) {
        opl2.setInstrument(i, instrument);                  // Assign the instrument to O
      }
    } break;
  }
  delay(100);
}

void oplProcess(uint8_t gates) {
  voct = analogRead(CV_VOCT);
  cv1 = analogRead(CV_CV1);
  cv2 = analogRead(CV_CV2);

  #if DEBUG == 1
  Serial.print("voct: ");
  Serial.print(voct);
  Serial.print(", ");
  Serial.print(cv1);
  Serial.print(", ");
  Serial.print(cv2);
  Serial.print("  |  ");
  Serial.print(gates, BIN);
  Serial.println("");
  #endif

  switch(currentMode) {
    case MODE_POLY: return doPoly(gates);
    case MODE_RHYTHM: return doRhythm(gates);
    case MODE_CHORD: return doChord(gates);
  }

}

bool doPlayNote(uint8_t gates, uint8_t voice, uint16_t on)
{
  if (isGate(voice) && !playedNote[voice]) //  && playNote[voice] > 16)
  {
    uint8_t octave = (on >> 8) & 0xFF;
    uint8_t note = on & 0xFF;

    playNote[voice] = 0;
    playedNote[voice] = true;
    opl2.playNote(voice, octave, note);
    return true;
  }
  return false;
}

bool shouldPlayNote(uint8_t gates, uint8_t voice)
{
  if (isGate(voice) && last_gate[voice])
  {
    playNote[voice]++;
  }
  return playNote[voice] > 16; // TODO: make this variable, or constant?
}

bool doStopNote(uint8_t gates, uint8_t voice)
{
  if (notGate(voice))
  {
    playedNote[voice] = false;
    playNote[voice] = 0;
    if (last_gate[voice])
    {
      opl2.setKeyOn(voice, false);
      return true;
    }
  }
  return false;
}

uint16_t voltToNote(uint16_t v)
{
  uint16_t semitone = map(v, 0, 1023, 0, 61);
  uint8_t octave = semitone / 12;
  uint8_t note = semitone - (octave * 12);
  octave += OCTAVE_BASE;
  return (octave << 8) | note;
}

void doPoly(uint8_t gates) 
{
  doPlayNote(gates, 0, voltToNote(voct));
  shouldPlayNote(gates, 0);
  doStopNote(gates, 0);
  last_gate[0] = isGate(0);

  doPlayNote(gates, 1, voltToNote(cv1));
  shouldPlayNote(gates, 1);
  doStopNote(gates, 1);
  last_gate[1] = isGate(1);

  doPlayNote(gates, 2, voltToNote(cv2));
  shouldPlayNote(gates, 2);
  doStopNote(gates, 2);
  last_gate[4] = isGate(2);
}

void doRhythm(uint8_t gates) 
{
  for (uint8_t voice = 0; voice < VOICES; voice++)
  {
    doPlayNote(gates, voice, voltToNote(voct));
    shouldPlayNote(gates, voice);
    doStopNote(gates, voice);
    last_gate[voice] = isGate(voice);
  }
}

void doChord(uint8_t gates)
{
  uint16_t on = voltToNote(voct);
  if(doPlayNote(gates, 0, on)) {
    uint8_t octave = (on >> 8) & 0xFF;
    uint8_t note = on & 0xFF;
    uint8_t third = note + 4;
    if(note > 11) {
      third -= 11;
      octave++;
    }
    opl2.playNote(1, octave, third);

    uint8_t fifth = third + 3;
    if(note > 11) {
      fifth -= 11;
      octave++;
    }
    opl2.playNote(3, octave, fifth);
  }
  shouldPlayNote(gates, 0);
  if(doStopNote(gates, 0)) {
    opl2.setKeyOn(1, false);
    opl2.setKeyOn(2, false);
  }
  last_gate[0] = isGate(0);
}

void setInstrument(uint8_t index)
{
  for (int i = 0; i < VOICES; i++)
  {
    int8_t load = index + (currentMode == MODE_RHYTHM ? (i * 4) : 0);
    if (load > 127)
    {
      load -= 127;
    }
    const unsigned char *_inst = midiInstruments[load];
    Instrument instrument = opl2.loadInstrument(_inst); // Load a piano instrument.
    opl2.setInstrument(i, instrument);                  // Assign the instrument to OPL2 channel 0.
    // opl2.setVibrato(i, CARRIER, true);
  }

  attack = opl2.getAttack(0, CARRIER);
  decay = opl2.getDecay(0, CARRIER);
  sustain = opl2.getSustain(0, CARRIER);
  release = opl2.getRelease(0, CARRIER);
  tremelo = opl2.getTremolo(0, CARRIER);
  vibrato = opl2.getVibrato(0, CARRIER);
}

void nextInstrument() {
  currentInstrument++;
  if (currentInstrument < 0)
  {
    currentInstrument = 0;
  }
  setInstrument(currentInstrument);
}

void prevInstrument() {
  currentInstrument--;
  if (currentInstrument < 0)
  {
    currentInstrument = 127;
  }
  setInstrument(currentInstrument);
}

void updateADSR(int8_t byRef)
{
  switch (select_menu)
  {
  case 0:
  { // attack
    attack += byRef;
    if (attack > 15)
      attack = 15;
    if (attack < 0)
      attack = 0;
    opl2.setAttack(0, CARRIER, attack);
  }
  break;
  case 1:
  { // decay
    decay += byRef;
    if (decay > 15)
      decay = 15;
    if (decay < 0)
      decay = 0;
    opl2.setDecay(0, CARRIER, decay);
  }
  break;
  case 2:
  { // sustain
    sustain += byRef;
    if (sustain > 15)
      sustain = 15;
    if (sustain < 0)
      sustain = 0;
    opl2.setSustain(0, CARRIER, sustain);
  }
  break;
  case 3:
  { // release
    release += byRef;
    if (release > 15)
      release = 15;
    if (release < 0)
      release = 0;
    opl2.setRelease(0, CARRIER, release);
  }
  break;
  case 4:
  { 
    tremelo = !tremelo;
    opl2.setTremolo(0, CARRIER, tremelo);
  }
  break;
  case 5:
  { 
    vibrato = !vibrato;
    opl2.setVibrato(0, CARRIER, vibrato);
  }
  break;
  }
}

#endif