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
#include <midi_instruments.h>
#include <Bounce2.h>
#include <RotaryEncoder.h>

#include "config.h"

OPL2 opl2;
uint16_t voct = 0; // 0-1023
uint16_t mod = 0;
uint16_t last_mod = 0;
byte last_inputs = B00000000;
byte playNote[VOICES] = {0, 0, 0, 0, 0, 0};
bool playedNote[VOICES] = { false, false, false, false, false, false, };
bool last_gate[VOICES] = { false, false, false, false, false, false, };
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

#define isGate(bit) !(((gates) >> (bit)) & 0x01)
#define notGate(bit) (((gates) >> (bit)) & 0x01)

// Bounce presetNext = Bounce();
// Bounce presetPrev = Bounce();
Bounce encoderSwitch = Bounce();
RotaryEncoder *encoder = nullptr;

int oldPosition = -999;
int newPosition = -999;
int i = 0;
int8_t select_menu = 0; //

void setInstrument();
void nextInstrument();
void prevInstrument();
void updateInstrument();
void updateADSR(int8_t byRef);
void encoderISR();
void updateEncoder();
byte shift();

void setup()
{
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

#if DEBUG == 1
  Serial.begin(115200);
  Serial.print("OPL2 Eurorack Module - ");
  Serial.println(VERSION);
#endif

  // setup 74hc165 pins
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_CLKIN, OUTPUT);
  pinMode(SHIFT_LOAD, OUTPUT);
  pinMode(SHIFT_DATA, INPUT);

  // presetNext.attach(B_PRESET_NEXT, INPUT_PULLUP);
  // presetNext.interval(5);

  // presetPrev.attach(B_PRESET_PREV, INPUT_PULLUP);
  // presetPrev.interval(5);

  encoderSwitch.attach(ENCODER_SW, INPUT_PULLUP);
  encoderSwitch.interval(5);

  encoder = new RotaryEncoder(ENCODER_1, ENCODER_2, ENCODER_LATCH);
  attachInterrupt(digitalPinToInterrupt(ENCODER_1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_2), encoderISR, CHANGE);
}

void loop()
{
  byte gates = shift();

  bool presetPrev = !bitRead(gates, CV_PREV);
  bool lastPresetPrev = bitRead(last_inputs, CV_PREV);

  bool presetNext = bitRead(gates, CV_NEXT);
  bool lastPresetNext = bitRead(last_inputs, CV_NEXT);

  voct = analogRead(CV_VOCT);
  mod = analogRead(CV_MOD);

#if DEBUG == 1
  float voltage = voct * (5.0 / 1023.0);
#endif

  uint16_t semitone = map(voct, 0, 1023, 0, 61);
  uint8_t octave = semitone / 12;
  uint8_t note = semitone - (octave * 12);
  octave += OCTAVE_BASE;

#if DEBUG == 1
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

  last_mod = mod;
  last_inputs = gates;

  updateEncoder();

  if (presetNext && !lastPresetNext) 
    nextInstrument();

  if (presetPrev && !lastPresetPrev)
    prevInstrument();

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

byte shift()
{
  digitalWrite(SHIFT_LOAD, LOW);
  delayMicroseconds(5);
  digitalWrite(SHIFT_LOAD, HIGH);
  delayMicroseconds(5);

  digitalWrite(SHIFT_CLK, HIGH);
  digitalWrite(SHIFT_CLKIN, LOW);
  byte v = shiftIn(SHIFT_DATA, SHIFT_CLK, LSBFIRST);
  digitalWrite(SHIFT_CLKIN, HIGH);

  return v;
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