#include <RotaryEncoder.h>

#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG 1
#define DEBUG_VERBOSE 0
#define VERSION "0.0.3-alpha"
#define CV_VOCT A0
#define CV_CV1 A1
#define CV_CV2 A2
// #define CV_GATE 12

// 74hc165 pins       Nano                  | 74hc165
#define SHIFT_CLKIN 4 // Clock Inhibit  | 15
#define SHIFT_LOAD 6  // Shift/Load     | 1
#define SHIFT_CLK 5   // Clock          | 2
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
#define MODE_RYTHM 1

#endif