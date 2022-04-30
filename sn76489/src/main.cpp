#include <Arduino.h>
#include <SN76489.h>
#include "pitches.h"

#define DEBUG 1
#define VERSION "0.0.3-alpha"
#define CV_VOCT A0
#define CV_MOD A1
#define CV_GATE 12

#define FREQUENCY 4000000.0
#define PIN_NotWE 10

// direct pin to pin setup
#define PIN_D0 2
#define PIN_D1 3
#define PIN_D2 4
#define PIN_D3 5
#define PIN_D4 6
#define PIN_D5 7
#define PIN_D6 8
#define PIN_D7 9
SN76489 chip = SN76489(PIN_NotWE, PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7, FREQUENCY);

// 595 setup
// #define PIN_SER   7
// #define PIN_LATCH 6
// #define PIN_CLK   5
// SN76489 chip = SN76489(PIN_NotWE, PIN_SER, PIN_LATCH, PIN_CLK, FREQUENCY);

/***************************************************************************
***** Melody and tempo data                                            *****
***************************************************************************/
uint16_t melody[] = {NOTE_G4, NOTE_F4, NOTE_AS4, NOTE_C5, NOTE_F5, NOTE_D5};
uint8_t tempo[] = {4, 4, 4, 8, 8, 4};

uint16_t voct = 0; // 0-1023
uint16_t mod = 0;
uint16_t last_mod = 0;
byte playNote = 0;
bool playedNote = false;
bool last_gate = false;
float VOLTAGE_OFFSET = 0.01;
uint8_t OCTAVE_BASE = 1;

void setup()
{
  // immediately quiet all channels
  chip.setAttenuation(0, 0xF);
  chip.setAttenuation(1, 0xF);
  chip.setAttenuation(2, 0xF);
  chip.setAttenuation(3, 0xF);

  Serial.begin(9600);
  Serial.println("SN76489");
}

void loop()
{
  bool gate = digitalRead(CV_GATE);
  voct = analogRead(CV_VOCT);
  mod = analogRead(CV_MOD);

  if (gate && !playedNote && playNote > 64)
  {
    playNote = 0;
    playedNote = true;
    #if DEBUG == 1
    float voltage = voct * (5.0 / 1023.0);
    #endif

    uint16_t semitone = map(voct, 0, 1023, 0, 61);
    uint8_t octave = semitone / 12;
    uint8_t note = semitone - (octave * 12);
    // uint8_t fifth_octave = octave;
    // uint8_t fifth = note + 7;
    // if (note > 12)
    // {
    //   fifth_octave++;
    //   fifth -= 12;
    // }
    octave += OCTAVE_BASE;

    uint16_t frequency = voltPerOctave[octave][note];

    #if DEBUG == 1
    Serial.print("V/OCT: ");

    Serial.print(semitone);
    Serial.print(", ");
    Serial.print(octave);
    Serial.print(", ");
    Serial.print(note);
    Serial.print(", ");
    Serial.print(frequency);
    Serial.print(", ");
    Serial.print(mod);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.println(voct);
    #endif

    // opl2.playNote(0, octave, note);
    for (byte i = 0; i < 3; i++)
    {
      chip.setAttenuation(i, 0x0);
      chip.setFrequency(i, frequency - (i * 6));
    }
  }

  if (gate && last_gate)
  {
    playNote++;
  }

  if (!gate)
  {
    playedNote = false;
    playNote = 0;
    if (last_gate)
    {
      // opl2.setKeyOn(0, false);
      for (byte i = 0; i < 4; i++)
      {
        chip.setAttenuation(i, 0xF);
      }
    }
  }

  if (mod < 512 && last_mod > 512)
  {
    // opl2.setDeepTremolo(tremelo);
    // opl2.setDeepTremolo(vibrato);
  }

  last_gate = gate;
  last_mod = mod;
}