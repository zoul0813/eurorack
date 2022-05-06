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
void doStopNote(uint8_t gates, uint8_t voice);
void shouldPlayNote(uint8_t gates, uint8_t voice);
void doPlayNote(uint8_t gates, uint8_t voice, uint16_t on);
void doRhythm(uint8_t gates);
void doPoly(uint8_t gates);
uint16_t voltToNote(uint16_t v);
void setInstrument();
void nextInstrument();
void prevInstrument();
void updateInstrument();
void updateADSR(int8_t byRef);

void oplInit() {
  opl2.begin();
  setInstrument();
  Instrument instrument = opl2.loadInstrument(INSTRUMENT_SYNDRUM); // Load a piano instrument.
  opl2.setInstrument(1, instrument);                  // Assign the instrument to OPL2 channel 0.

  instrument = opl2.loadInstrument(INSTRUMENT_SHANNAI); // Load a piano instrument.
  opl2.setInstrument(2, instrument);                  // Assign the instrument to OPL2 channel 0.

  instrument = opl2.loadInstrument(INSTRUMENT_TAIKO); // Load a piano instrument.
  opl2.setInstrument(3, instrument);                  // Assign the instrument to OPL2 channel 0.

  instrument = opl2.loadInstrument(INSTRUMENT_WOODBLOK); // Load a piano instrument.
  opl2.setInstrument(4, instrument);                  // Assign the instrument to OPL2 channel 0.

  instrument = opl2.loadInstrument(INSTRUMENT_STEELDRM); // Load a piano instrument.
  opl2.setInstrument(5, instrument);                  // Assign the instrument to OPL2 channel 0.
  delay(100);
}

void oplProcess(uint8_t gates) {
  voct = analogRead(CV_VOCT);
  cv1 = analogRead(CV_CV1);
  cv2 = analogRead(CV_CV2);

  Serial.print(voct);
  Serial.print(", ");
  Serial.print(cv1);
  Serial.print(", ");
  Serial.print(cv2);
  Serial.println("");

#if DEBUG_VERBOSE == 1
  float voltage = voct * (5.0 / 1023.0);
#endif

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

  switch(currentMode) {
    case MODE_POLY: return doPoly(gates);
    case MODE_RYTHM: return doRhythm(gates);
  }

}

void doPlayNote(uint8_t gates, uint8_t voice, uint16_t on)
{
  if (isGate(voice) && !playedNote[voice]) //  && playNote[voice] > 16)
  {
    uint8_t octave = (on >> 8) & 0xFF;
    uint8_t note = on & 0xFF;
    Serial.print("Octave: ");
    Serial.print(octave);
    Serial.print(", Note: ");
    Serial.println(note);

    playNote[voice] = 0;
    playedNote[voice] = true;
    opl2.playNote(voice, octave, note);
  }
}

void shouldPlayNote(uint8_t gates, uint8_t voice)
{
  if (isGate(voice) && last_gate[voice])
  {
    playNote[voice]++;
  }
}

void doStopNote(uint8_t gates, uint8_t voice)
{
  if (notGate(voice))
  {
    playedNote[voice] = false;
    playNote[voice] = 0;
    if (last_gate[voice])
    {
      opl2.setKeyOn(voice, false);
    }
  }
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

  doPlayNote(gates, 4, voltToNote(cv2));
  shouldPlayNote(gates, 4);
  doStopNote(gates, 4);
  last_gate[4] = isGate(4);
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

void setInstrument()
{
  for (int i = 0; i < VOICES; i++)
  {
    int8_t load = currentInstrument + (i * 3);
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

#if DEBUG == 1
  Serial.print("Instrument: ");
  Serial.print(currentInstrument);
  // Serial.print(" (");
  // Serial.print(midiInstrumentNames[currentInstrument]);
  Serial.print(") ");
  Serial.print(" - A: ");
  Serial.print(attack);
  Serial.print(", D: ");
  Serial.print(decay);
  Serial.print(", S: ");
  Serial.print(sustain);
  Serial.print(", R: ");
  Serial.print(release);
  Serial.print(", T: ");
  Serial.print(tremelo);
  Serial.print(", V: ");
  Serial.print(vibrato);
  Serial.println("");
#endif
}

void nextInstrument() {
  currentInstrument++;
  if (currentInstrument < 0)
  {
    currentInstrument = 0;
  }
  setInstrument();
}

void prevInstrument() {
  currentInstrument--;
  if (currentInstrument < 0)
  {
    currentInstrument = 127;
  }
  setInstrument();
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
#if DEBUG == 1
    Serial.print("Attack: ");
    Serial.println(attack);
#endif
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
#if DEBUG == 1
    Serial.print("Decay: ");
    Serial.println(decay);
#endif
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
#if DEBUG == 1
    Serial.print("Sustain: ");
    Serial.println(sustain);
#endif
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
#if DEBUG == 1
    Serial.print("Release: ");
    Serial.println(release);
#endif
  }
  break;
  case 4:
  { 
    tremelo = !tremelo;
    opl2.setTremolo(0, CARRIER, tremelo);
#if DEBUG == 1
    Serial.print("Tremelo: ");
    Serial.println(tremelo);
#endif
  }
  break;
  case 5:
  { 
    vibrato = !vibrato;
    opl2.setVibrato(0, CARRIER, vibrato);
#if DEBUG == 1
    Serial.print("Vibrato: ");
    Serial.println(vibrato);
#endif
  }
  break;
  }
}

#endif