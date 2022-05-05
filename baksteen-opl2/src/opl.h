#include <Arduino.h>
#include <OPL2.h>
#include <midi_instruments.h>
#include "config.h"
#include "globals.h"

OPL2 opl2;

// the +2 should be compiled out
#define isGate(bit) !(((gates) >> (bit)) & 0x01)
#define notGate(bit) (((gates) >> (bit)) & 0x01)

void oplInit();
void oplProcess(uint8_t gates);
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
  for (uint8_t voice = 0; voice < VOICES; voice++)
  {

    if (isGate(voice) && !playedNote[voice]) //  && playNote[voice] > 16)
    {
      playNote[voice] = 0;
      playedNote[voice] = true;
      opl2.playNote(voice, octave, note);
    }

    if (isGate(voice) && last_gate[voice])
    {
      playNote[voice]++;
    }

    if (notGate(voice))
    {
      playedNote[voice] = false;
      playNote[voice] = 0;
      if (last_gate[voice])
      {
        opl2.setKeyOn(voice, false);
      }
    }
    last_gate[voice] = isGate(voice);
  }

  if (mod < 512 && last_mod > 512)
  {
    #if DEBUG == 1
      Serial.println("Deep Vibrato/Tremolo");
    #endif
    opl2.setDeepTremolo(tremelo);
    opl2.setDeepVibrato(vibrato);
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

