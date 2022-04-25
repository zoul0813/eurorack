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
#include <Bounce2.h>
#include <RotaryEncoder.h>
#include <avdweb_AnalogReadFast.h>


#define DEBUG 1
#define CV_VOCT A0
#define CV_GATE 12

#define B_PRESET_NEXT 5
#define B_PRESET_PREV 6

#define ENCODER_1 2
#define ENCODER_2 3
#define ENCODER_SW 7
#define ENCODER_LATCH RotaryEncoder::LatchMode::FOUR3

#define VOICES 4

OPL2 opl2;
uint16_t voct = 2; // 0-1023
byte playNote = 0;
bool playedNote = false;
bool last_gate = false;
float VOLTAGE_OFFSET = 0.01;
uint8_t OCTAVE_BASE = 2;
uint8_t channel = 0;
int8_t currentInstrument = 0;

uint8_t attack = 0;
uint8_t decay = 0;
uint8_t sustain = 0;
uint8_t release = 0;

Bounce presetNext = Bounce();
Bounce presetPrev = Bounce();
Bounce encoderSwitch = Bounce();
RotaryEncoder *encoder = nullptr;

int oldPosition = -999;
int newPosition = -999;
int i = 0;
int8_t select_menu = 0; //

void setInstrument();
void updateEncoder();
void updateADSR(int8_t byRef);

void setup()
{
  #if DEBUG == 1
  Serial.begin(9600);
  #endif

  presetNext.attach(B_PRESET_NEXT, INPUT_PULLUP);
  presetNext.interval(5);

  presetPrev.attach(B_PRESET_PREV, INPUT_PULLUP);
  presetPrev.interval(5);

  encoderSwitch.attach(ENCODER_SW, INPUT_PULLUP);
  encoderSwitch.interval(5);

  encoder = new RotaryEncoder(ENCODER_1, ENCODER_2, ENCODER_LATCH);
  attachInterrupt(digitalPinToInterrupt(ENCODER_1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_2), updateEncoder, CHANGE);

  opl2.begin();
  setInstrument();
}

void loop()
{
  bool gate = digitalRead(CV_GATE);
  voct = analogRead(CV_VOCT);
  presetNext.update();
  presetPrev.update();
  encoderSwitch.update();

  oldPosition = newPosition;
  newPosition = encoder->getPosition();
  RotaryEncoder::Direction encoderDirection = encoder->getDirection();

  if (presetNext.changed() && presetNext.read())
  {
    currentInstrument++;
    if (currentInstrument < 0)
    {
      currentInstrument = 0;
    }
    setInstrument();
  }

  if (presetPrev.changed() && presetPrev.read())
  {
    currentInstrument--;
    if (currentInstrument < 0)
    {
      currentInstrument = 127;
    }
    setInstrument();
  }

  if (encoderSwitch.changed() && encoderSwitch.read())
  {
    select_menu++;
    if (select_menu > 3)
    {
      select_menu = 0;
    }

    #if DEBUG == 1
    switch(select_menu) {
      case 0: Serial.println("Attack"); break;
      case 1: Serial.println("Decay"); break;
      case 2: Serial.println("Sustain"); break;
      case 3: Serial.println("Release"); break;
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

  if (gate && !playedNote && playNote > 16)
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

    #if DEBUG == 1
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
    #endif

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
  }
  
  // playNote = gate && !last_gate;
  if(gate && last_gate) {
    playNote++;
  }

  if(!gate) {
    playedNote = false;
    playNote = 0;
  }

  last_gate = gate;
}

void updateEncoder()
{
  encoder->tick(); // just call tick() to check the state.
}

void setInstrument()
{
  const unsigned char *inst = midiInstruments[currentInstrument];
  #if DEBUG == 1
  Serial.print("Instrument: ");
  Serial.println(currentInstrument);
  #endif 
  Instrument piano = opl2.loadInstrument(inst); // Load a piano instrument.


  for (int i = 0; i < VOICES; i++)
  {
    opl2.setInstrument(i, piano); // Assign the instrument to OPL2 channel 0.
  }

  attack = opl2.getAttack(0, CARRIER);
  decay = opl2.getDecay(0, CARRIER);
  sustain = opl2.getSustain(0, CARRIER);
  release = opl2.getRelease(0, CARRIER);
}

void updateADSR(int8_t byRef) {
  switch(select_menu) {
    case 0: { // attack
      attack += byRef;
      if(attack > 16) attack = 16;
      if(attack < 0) attack = 0;
      opl2.setAttack(0, CARRIER, attack);
      #if DEBUG == 1
      Serial.print("Attack: ");
      Serial.println(attack);
      #endif
    } break;
    case 1: { // decay 
      decay += byRef;
      if(decay > 15) decay = 16;
      if(decay < 0) decay = 0;
      opl2.setDecay(0, CARRIER, decay);
      #if DEBUG == 1
      Serial.print("Decay: ");
      Serial.println(decay);
      #endif 
    } break;
    case 2: { // sustain
      sustain += byRef;
      if(sustain > 16) sustain = 16;
      if(sustain < 0) sustain = 0;
      opl2.setSustain(0, CARRIER, sustain);
      #if DEBUG == 1
      Serial.print("Sustain: ");
      Serial.println(sustain);
      #endif
    }  break;
    case 3: { // release
      release += byRef;
      if(release > 16) release = 16;
      if(release < 0) release = 0;
      opl2.setRelease(0, CARRIER, release);
      #if DEBUG == 1
      Serial.print("Release: ");
      Serial.println(release);
      #endif 
    } break;
  }
}
