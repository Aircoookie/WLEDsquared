//Matrix effect modes

#include "WS2812FX.h"

uint16_t w = 64, h = 32; //matrix width and height

void WS2812FX::map1Dto2D(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
  if (_realtimeMode) 
  {
    mode2D_spectrum(i,r,g,b);
  } else
  {
    switch (_1Dto2Dmapping)
    {
      case P_MODE_FRAME: mode2D_frame(i,r,g,b); break;
      case P_MODE_HEART: mode2D_heart(i,r,g,b); break;
      default: mode2D_frame(i,r,g,b);
    }
  }
}

//28+px heart LUT
typedef struct coord {
  byte x;
  byte y;
};

void WS2812FX::mode2D_frame(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
  if (i < w)
  {
    bus->SetPixelColor(i, 0, r, g, b);
  } else if (i < w + h -1)
  {
    bus->SetPixelColor(w -1, i - w +1, r, g, b);
  } else if (i < w*2 + h -2)
  {
    bus->SetPixelColor(w -1 -(i -w -h +1), h -1, r, g, b);
  } else if (i < w*2 + h*2 -3)
  {
    bus->SetPixelColor(0, h-1 - (i -(w*2 + h -2)), r, g, b);
  }
}

const coord heart28_table[] = {{13,27},{13,26},{12,25},{12,24},{11,23},{11,22},
                               {10,21},{10,20},{ 9,19},{ 8,18},{ 7,17},{ 6,16},
                               { 5,15},{ 4,14},{ 3,13},{ 2,12},{ 1,11},{ 1,10},
                               { 0, 9},{ 0, 8},{ 0, 7},{ 1, 6},{ 1, 5},{ 2, 4},
                               { 2, 3},{ 3, 2},{ 4, 1},{ 5, 1},{ 6, 0},{ 7, 0},
                               { 8, 1},{ 9, 1},{10, 2},{11, 3},{11, 4},{12, 5},
                               {12, 6},{13, 7},{13, 8}};

void WS2812FX::mode2D_heart(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
  if (w > 27 && h > 27)
  {
    //28x28 heart has 39*2 = 78 LEDs
    byte x_offset = w /2 -14;
    byte y_offset = h /2 -14;
    if (i < 39)
    {
      bus->SetPixelColor(heart28_table[i].x + x_offset, heart28_table[i].y + y_offset, r,g,b);
    } else if (i < 78)
    {
      bus->SetPixelColor(27 - heart28_table[77-i].x + x_offset, heart28_table[77-i].y + y_offset, r,g,b);
    }
  }
}

void WS2812FX::mode2D_spectrum(uint16_t i, uint8_t r, uint8_t g, uint8_t b)
{
  static byte peak[PxMATRIX_MAX_WIDTH]; //TODO handle this with less ram usage
  static byte cheight[PxMATRIX_MAX_WIDTH];
  static byte pdelay[PxMATRIX_MAX_WIDTH];
  
  if (i >= w) return;

  uint32_t col1 = 0;
  uint16_t half = w /2;
  if (i < half)
  {
    col1 = color_from_palette(255- i*(256/half),false,PALETTE_SOLID_WRAP,0);
  } else
  {
    col1 = color_from_palette((i - half)*(256/half),false,PALETTE_SOLID_WRAP,0);
  }
  uint8_t r1 = col1 >> 16 & 0xFF;
  uint8_t g1 = col1 >>  8 & 0xFF;
  uint8_t b1 = col1       & 0xFF;

  uint8_t r2 = SEGMENT.colors[1] >> 16 & 0xFF;
  uint8_t g2 = SEGMENT.colors[1] >>  8 & 0xFF;
  uint8_t b2 = SEGMENT.colors[1]       & 0xFF;
  if (r2 == 0 && g2 == 0 && b2 == 0) g2 = 255;

  byte newh = max(r,max(g,b))/(256/h);
  if (newh > cheight[i])
  {
    for (byte p = cheight[i]; p <= newh; p++)
    {
      bus->SetPixelColor(i,h - p, r1,g1,b1);
    }
    if (newh > peak[i])
    {
      pdelay[i] = 15;
      peak[i] = newh;
      bus->SetPixelColor(i,h -1 - newh, r2,g2,b2);
    }
    cheight[i] = newh;
  } else if (newh < cheight[i])
  {
    for (byte p = newh +1; p <= cheight[i]; p++)
    {
      bus->SetPixelColor(i,h - p, 0,0,0);
    }
    cheight[i] = newh;
  }
  if (pdelay[i] > 0)
  {
    pdelay[i]--;
  } else
  {
    bus->SetPixelColor(i,h -1 - peak[i], 0,0,0);
    if (peak[i] > 0)
    {
      if (peak[i] > cheight[i]) peak[i]--;
      bus->SetPixelColor(i,h -1 - peak[i], r2,g2,b2);
    }
    
    pdelay[i] = 5;
  }
}
