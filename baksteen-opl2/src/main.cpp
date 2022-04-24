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
#include <OPL2.h>
#include <midi_instruments.h>

#define VOICES 4

OPL2 opl2;
uint16_t voct = 2; // 0-1023
uint16_t last_voctP = 2;
uint16_t last_voctM = 2;
uint32_t last_note = 0;
float VOLTAGE_OFFSET = 0.01;
uint8_t OCTAVE_BASE = 2;
uint8_t channel = 0;

void setup()
{
  Serial.begin(9600);

  opl2.begin();
  Instrument piano = opl2.loadInstrument(INSTRUMENT_CRYSTAL);      // Load a piano instrument.
  for(int i = 0; i < VOICES; i++) {
    opl2.setInstrument(i, piano);                                   // Assign the instrument to OPL2 channel 0.
    opl2.setVolume(i, CARRIER, 0x00);
  }
}

void loop()
{
  uint32_t ms = millis();

  voct = analogRead(A1);
  float voltage = voct * (5.0 / 1023.0);

  uint16_t semitone = map(voct, 0, 1023, 0, 61);
  uint8_t octave = semitone / 12;
  uint8_t note = semitone - (octave * 12);
  uint8_t fifth_octave = octave;
  uint8_t fifth = note + 7;
  if(note > 12) {
    fifth_octave++;
    fifth -= 12;
  }
  octave += OCTAVE_BASE; 
  
  if (voct > 0 && (voct < last_voctM || voct > last_voctP))
  {
    last_voctM = voct - 2;
    last_voctP = voct + 2;
    Serial.print("V/OCT: ");

    Serial.print(semitone);
    Serial.print(", ");
    Serial.print(octave);
    Serial.print(", ");
    Serial.print(note);
    Serial.print(", ");
    Serial.print(channel);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.println(voct);

    // for(int i = 0; i < VOICES; i++) {
    //   opl2.playNote(i, octave, note);
    // }

    // opl2.playNote(channel, octave, note);
    // channel++;
    // if(channel >= VOICES) {
    //   channel = 0;
    // }

    opl2.playNote(0, octave, note);
    // opl2.playNote(1, fifth_octave, fifth);

    last_note = ms;
  }
}
