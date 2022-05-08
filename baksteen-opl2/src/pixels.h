#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "globals.h"

#ifndef __PIXELS_H
#define __PIXELS_H

void pixelsInit();
void updatePixels(uint8_t inputs);

void pixelsInit()
{
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.setBrightness(32);
  pixels.show();
}

void updatePixels(uint8_t inputs)
{
  pixels.clear();
  for(uint8_t i = 0; i < NEOPIXEL_NUM; i++) {
    if(!bitRead(inputs, i)) {
      pixels.setPixelColor(i, currentMode == MODE_RHYTHM ? NEOPIXEL_RHYTHM : NEOPIXEL_POLY);
    }
  }

  switch(currentMode) {
    case MODE_POLY: {
      pixels.setPixelColor(0, NEOPIXEL_POLY);
    } break;
    case MODE_RHYTHM: {
      pixels.setPixelColor(0, NEOPIXEL_RHYTHM);
    } break;
    case MODE_CHORD: {
      pixels.setPixelColor(0, NEOPIXEL_CHORD);
    };
  }

  pixels.show(); // Send the updated pixel colors to the hardware.
}

#endif