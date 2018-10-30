//interface WLED with chinese 64x32 multiplex matrix - PRE ALPHA DEMO
#ifndef NpbWrapper_h
#define NpbWrapper_h

//save memory
#define PxMATRIX_MAX_HEIGHT 32
#define PxMATRIX_MAX_WIDTH 64

#include <Ticker.h> //ESP8266 ONLY
#include <PxMatrix.h>
#include <NeoPixelBus.h>
// The library for controlling the LED Matrix
// Needs to be manually downloaded and installed
// https://github.com/2dom/PxMatrix

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

enum NeoPixelType
{
  NeoPixelType_None = 0,
  NeoPixelType_Grb  = 1,
  NeoPixelType_Grbw = 2,
  NeoPixelType_End  = 3
};

class NeoPixelWrapper
{
public:
  NeoPixelWrapper()
  {

  }

  ~NeoPixelWrapper()
  {
    cleanup();
  }

  void Begin(NeoPixelType type, uint16_t countPixels);

  void Show();
  bool CanShow() const;

  void SetPixelColor(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
  void SetPixelColor(uint16_t indexPixel, uint8_t r, uint8_t g, uint8_t b);
  void SetBrightness(byte b);

  RgbColor GetPixelColor(uint16_t indexPixel) const;
  RgbwColor GetPixelColorRgbw(uint16_t indexPixel) const;

  void Clear(void);

private:
  void cleanup();
};
#endif
