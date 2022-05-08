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

  pixels.show(); // Send the updated pixel colors to the hardware.
}

#endif