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
#include <SPI.h>
#include <OPL2.h>
#include <instruments.h>

#define TRIG_LEN 30 // 30ms

OPL2 opl2;

bool bassPrev = false;
bool snarePrev = false;
bool tomPrev = false;
bool cymbalPrev = false;
bool hatsPrev = false;

void setup()
{
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  Serial.begin(9600);

  opl2.begin();

  // Set percussion mode and load instruments.
  Instrument bass = opl2.loadInstrument(INSTRUMENT_BDRUM2);
  Instrument snare = opl2.loadInstrument(INSTRUMENT_RKSNARE1);
  Instrument tom = opl2.loadInstrument(INSTRUMENT_TOM2);
  Instrument cymbal = opl2.loadInstrument(INSTRUMENT_CYMBAL1);
  // Instrument cymbal = opl2.loadInstrument(INSTRUMENT_HIHAT1);
  Instrument hihat = opl2.loadInstrument(INSTRUMENT_HIHAT2);

  opl2.setPercussion(true);
  opl2.setDrumInstrument(bass, DRUM_BASS);
  opl2.setDrumInstrument(snare, DRUM_SNARE);
  opl2.setDrumInstrument(tom, DRUM_TOM);
  opl2.setDrumInstrument(cymbal, DRUM_CYMBAL);
  opl2.setDrumInstrument(hihat, DRUM_HI_HAT);

  // Set octave and frequency for bass drum.
  opl2.setBlock(6, 4);
  opl2.setFNumber(6, opl2.getNoteFNumber(NOTE_C));

  // Set octave and frequency for snare drum and hi-hat.
  opl2.setBlock(7, 3);
  opl2.setFNumber(7, opl2.getNoteFNumber(NOTE_C));
  // Set low volume on hi-hat
  opl2.setVolume(7, OPERATOR1, 16);

  // Set octave and frequency for tom tom and cymbal.
  opl2.setBlock(8, 3);
  opl2.setFNumber(8, opl2.getNoteFNumber(NOTE_A));

  opl2.setDrums(true, true, false, true, false);
  delay(200);
  opl2.setDrums(false, false, false, false, false);
}

void loop()
{
  bool bass = digitalRead(A1);
  bool snare = digitalRead(A2);
  bool tom = digitalRead(A3);
  bool cymbal = digitalRead(A4);
  bool hats = digitalRead(A5);

  Serial.print(bass);
  Serial.print(", ");
  Serial.print(snare);
  Serial.print(", ");
  Serial.print(tom);
  Serial.print(", ");
  Serial.print(cymbal);
  Serial.print(", ");
  Serial.println(hats);

  bool update = false;

  update = (bass && !bassPrev) 
    || (snare && !snarePrev)
    || (tom && !tomPrev)
    || (cymbal && !cymbalPrev)
    || (hats && !hatsPrev);

  if (update)
  {
    opl2.setDrums(bass, snare, tom, cymbal, hats);
    delay(10);
  }

  bassPrev = bass;
  snarePrev = snare;
  tomPrev = tom;
  cymbalPrev = cymbal;
  hatsPrev = hats;
}
