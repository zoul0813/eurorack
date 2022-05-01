#include <Arduino.h>
#include <SN76489.h>
#include "pitches.h"

#define DEBUG 1
#define VERSION "0.0.3-alpha"
#define CV_VOCT A0
#define CV_MOD1 A1
#define CV_MOD2 A2
#define CV_MOD3 A3
#define CV_GATE 4

#define FREQUENCY 4000000.0
#define PIN_NotWE 9

// direct pin to pin setup
// #define PIN_D0 2
// #define PIN_D1 3
// #define PIN_D2 4
// #define PIN_D3 5
// #define PIN_D4 6
// #define PIN_D5 7
// #define PIN_D6 8
// #define PIN_D7 9
// SN76489 chip = SN76489(PIN_NotWE, PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7, FREQUENCY);

// 595 setup
#define PIN_SER   10
#define PIN_LATCH 8
#define PIN_CLK   7
SN76489 chip = SN76489(PIN_NotWE, PIN_SER, PIN_LATCH, PIN_CLK, FREQUENCY);

/***************************************************************************
***** Melody and tempo data                                            *****
***************************************************************************/
uint16_t melody[] = {NOTE_G4, NOTE_F4, NOTE_AS4, NOTE_C5, NOTE_F5, NOTE_D5};
uint8_t tempo[] = {4, 4, 4, 8, 8, 4};

uint16_t voct = 0; // 0-1023
uint16_t mod1 = 0;
uint16_t mod2 = 0;
uint16_t mod3 = 0; 
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

  Serial.begin(115200);
  Serial.println("SN76489");
}

void loop()
{
  bool gate = digitalRead(CV_GATE);
  voct = analogRead(CV_VOCT);
  mod1 = analogRead(CV_MOD1);
  mod2 = analogRead(CV_MOD2);
  mod3 = analogRead(CV_MOD3);

  if (gate) {
    if(playNote > 64  && !playedNote)
    {
      // playNote = 0;
      playedNote = true;
      #if DEBUG == 1
      float voltage = voct * (5.0 / 1023.0);
      #endif

      uint16_t semitone = map(voct, 0, 1023, 0, 61);
      uint8_t octave = semitone / 12;
      uint8_t note = semitone - (octave * 12);
      octave += OCTAVE_BASE;
      uint8_t detune = map(mod1, 0, 1023, 0, 64);
      uint8_t noise = map(mod2, 0, 1023, 0, 9);

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
      Serial.print(mod1);
      Serial.print(", ");
      Serial.print(detune);
      Serial.print(", ");
      Serial.print(mod2);
      Serial.print(", ");
      Serial.print(noise);
      Serial.print(", ");
      Serial.print(voltage);
      Serial.print(", ");
      Serial.println(voct);
      #endif

      // order matters, as the last register is latched, for frequency sweep
      // should sweep 3rd oscillator which will adjust noise and main frequency
      if(noise > 0) {
        chip.setAttenuation(3, 0xF);
        if(noise < 5) {
          chip.setNoise(1, noise - 1);
        } else {
          chip.setNoise(0, noise - 1);
        }
      }

      // first osc
      chip.setAttenuation(0, 0x0);
      chip.setFrequency(0, frequency - (2 * detune));

      // second osc
      chip.setAttenuation(1, 0x0);
      chip.setFrequency(1, frequency - (1 * detune));

      // third osc // controls noise frequency
      chip.setAttenuation(2, 0x0);
      chip.setFrequency(2, frequency);
    }
  
    if(mod3 > 0 && playNote > 96 && playedNote) {
      uint8_t sweep = map(mod3, 0, 1023, 0, 63);
      // sweep = (sweep >> 2) & ~(0x00111111);
      Serial.print("S: ");
      Serial.println(sweep, BIN);
      chip.SendByteToSoundChip(sweep);
    }
  }

  if (gate && last_gate)
  {
    // don't overflow ... 
    if(playNote < 254) playNote++;
  }

  if (!gate)
  {
    playedNote = false;
    playNote = 0;
    if (last_gate)
    {
      for (byte i = 0; i < 4; i++)
      {
        chip.setAttenuation(i, 0xF);
      }
    }
  }

  last_gate = gate;
}