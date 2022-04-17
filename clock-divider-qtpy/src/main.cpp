#include <Arduino.h>

#include "SAMDTimerInterrupt.h"
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG 0
#define KNOB_1 1       // A1
#define KNOB_2 2       // A2
#define CLOCK_IN 11    // SCK
#define CLOCK_OUT 9    // MISO
#define GATE_OUT1 4    // SDA
#define GATE_OUT2 5    // SCL
#define SELECT_SW 3    // A3
#define CLOCK_LENGTH 1 // how long to hold the clock high for, in millis

Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);
SAMDTimer ITimer0(TIMER_TC3);
Bounce tempoUpBounce = Bounce();
Bounce tempoDownBounce = Bounce();

bool clockOut = false;
int tempo = 125;                                      // beats per minute
int ppq = 2;                                         // pulses per quarter
float msPulseWait = 1000 / ((tempo / 60) * ppq); // Tempo = 120, PPQ = 24
volatile unsigned long lastClock = 0;

void setup();
void loop();
float getPulseWaitTime(int tempo, int ppq);
void setTempo(int tempo);
void TimerHandler0();

void setup()
{
  pinMode(GATE_OUT1, INPUT_PULLUP); // CH1out
  pinMode(GATE_OUT2, INPUT_PULLUP); // CH2out

  pinMode(CLOCK_OUT, OUTPUT);       // internal_clock_out
  pinMode(SELECT_SW, INPUT_PULLUP); // SW
  pinMode(CLOCK_IN, INPUT);         // ext_clock_in

  Serial.begin(9600);

  pixels.begin(); // initialize the pixel
  // set the first pixel #0 to red
  pixels.setBrightness(196);
  pixels.setPixelColor(0, pixels.Color(0, 255, 255));
  // and write the data
  pixels.show();

  // setup internal tempo +/- buttons
  tempoDownBounce.attach(GATE_OUT1, INPUT_PULLUP); // USE INTERNAL PULL-UP
  tempoDownBounce.interval(5);                     // interval in ms

  tempoUpBounce.attach(GATE_OUT2, INPUT_PULLUP); // USE INTERNAL PULL-UP
  tempoUpBounce.interval(5);                     // interval in ms

#if DEBUG == 1
  Serial.print("msPulseWait = ");
  Serial.println(msPulseWait);
#endif

  setTempo(tempo);
}

void loop()
{
  int clockIn = digitalRead(CLOCK_IN); // read external clock signal

  tempoDownBounce.update();
  tempoUpBounce.update();

  long ms = millis();

  if (clockOut == true)
  {
    if (ms - lastClock > CLOCK_LENGTH)
    {
      digitalWrite(CLOCK_OUT, LOW);
    }
  }

  if (tempoDownBounce.changed())
  {
    // increase on press, update on release
    if (tempoDownBounce.read())
    {
      Serial.println("tempoDown: released");
      setTempo(tempo);
    }
    else
    {
      Serial.println("tempoDown: pressed");
      tempo--;
    }
  }

  if (tempoUpBounce.changed())
  {
    // increase on press, update on release
    if (tempoUpBounce.read())
    {
      setTempo(tempo);
    }
    else
    {
      tempo++;
    }
  }
}

void setTempo(int tempo)
{
  ITimer0.stopTimer();
  ITimer0.detachInterrupt();
  msPulseWait = getPulseWaitTime(tempo, ppq);
  Serial.print("Tempo: ");
  Serial.println(tempo);
  Serial.print("Pulse Wait: ");
  Serial.println(msPulseWait);
  if (ITimer0.attachInterrupt(msPulseWait, TimerHandler0))
  {
    lastClock = millis();
    Serial.print(F("Starting ITimer0 OK, millis() = "));
    Serial.println(lastClock);
  }
}

float getPulseWaitTime(int _tempo, int _ppq)
{
  Serial.print("getPulseWaitTime: ");
  Serial.print(_tempo);
  Serial.print(", ");
  Serial.println(_ppq);
  // (1000ms * 1000us) / ((tempo / 60s) * pulsesPerQuarter)

  // set the frequency ... ((tempo / 60s) * pulsesPerQuarter) * 1hZ
  return (float)((_tempo / 60.0f) * _ppq);
}

void TimerHandler0()
{
  lastClock = millis();
  // pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  // pixels.show();
  digitalWrite(CLOCK_OUT, HIGH);
  clockOut = true;
#if DEBUG == 1
  Serial.print("o");
#endif
}