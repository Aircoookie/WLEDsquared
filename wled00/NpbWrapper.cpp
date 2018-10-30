//interface WLED with chinese 64x32 multiplex matrix - PRE ALPHA DEMO
#include <Ticker.h> //ESP8266 ONLY

#include "NpbWrapper.h"
#include <PxMatrix.h>
#include <NeoPixelBus.h>

// The library for controlling the LED Matrix
// Needs to be manually downloaded and installed
// https://github.com/2dom/PxMatrix

// Pins for LED MATRIX
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_OE 2
#define P_D 12
#define P_E 0

Ticker display_ticker;


// PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
// PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX mDisplay(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);


// ISR for display refresh
void display_updater()
{
  mDisplay.display(70);
}

void NeoPixelWrapper::Begin(NeoPixelType type, uint16_t countPixels)
{
  cleanup();
  mDisplay.begin(16); //display type (1/2, 1/4, 1/8, 1/16 or 1/32)
  display_ticker.attach(0.002, display_updater);
}

void NeoPixelWrapper::Show()
{
  
}

bool NeoPixelWrapper::CanShow() const
{
  return true;
}

void NeoPixelWrapper::SetPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
  mDisplay.drawPixelRGB888(x, y, r, g, b);
}

void NeoPixelWrapper::SetBrightness(byte b)
{
  mDisplay.setBrightness(b);
}

RgbColor NeoPixelWrapper::GetPixelColor(uint16_t indexPixel) const
{
  //not supported by PxMatrix
  return 0;
}

RgbwColor NeoPixelWrapper::GetPixelColorRgbw(uint16_t indexPixel) const
{
  return 0;
}

void NeoPixelWrapper::Clear()
{
  mDisplay.clearDisplay();
}

void NeoPixelWrapper::cleanup()
{
  
}
