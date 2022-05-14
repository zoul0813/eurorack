#include <RotaryEncoder.h>

#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG 0
#define DEBUG_VERBOSE 0
#define VERSION "0.0.5-alpha"
#define CV_VOCT A0
#define CV_CV1 A1
#define CV_CV2 A2
// #define CV_GATE 12

// 74hc165 pins       Nano                  | 74hc165
#define SHIFT_CLKIN 4 // Clock Inhibit  | 15
#define SHIFT_LOAD 5  // Shift/Load     | 1
#define SHIFT_CLK 6   // Clock          | 2
#define SHIFT_DATA 12 // Data / Qh      | 7

// bits for the 74hc165 values
#define CV_NEXT 0
#define CV_PREV 1
#define CV_GATE1 2
#define CV_GATE2 3
#define CV_GATE3 4
#define CV_GATE4 5
#define CV_GATE5 6
#define CV_GATE6 7

// #define B_PRESET_NEXT 5
// #define B_PRESET_PREV 6

#define ENCODER_1 2
#define ENCODER_2 3
#define ENCODER_SW 7
#define ENCODER_LATCH RotaryEncoder::LatchMode::FOUR3

#define VOICES 6

#define MODE_POLY 0
#define MODE_RHYTHM 1
#define MODE_CHORD 2
#define MODE_DEFAULT MODE_RHYTHM

#define NEOPIXEL_PIN 0 // On Trinket or Gemma, suggest changing this to 1
#define NEOPIXEL_NUM 8 // Popular NeoPixel ring size

#define NEOPIXEL_RHYTHM pixels.Color(244, 27, 63) // pink
#define NEOPIXEL_POLY pixels.Color(27,244,63) // green?
#define NEOPIXEL_CHORD pixels.Color(27,63,244) // blue?

#endif