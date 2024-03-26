#ifndef _GxEPD2_Extra_H_
#define _GxEPD2_Extra_H_

#include <epd3c/GxEPD2_213c.h>

// SES电子价签所用的墨水屏来自https://www.pervasivedisplays.com/product/2-13-e-ink-display-spectra-r2-0/
// BUSY脚电平与微雪的2.13inch e-Paper HAT (B)相反, 其余大致相同
// 建议采用官方库驱动https://github.com/rei-vilo/PDLS_EXT3_Basic, 但本项目为了兼容其他墨水屏就魔改GxEPD2了
class GxEPD2_213_EXT3 : public GxEPD2_213c // 型号为xE2213CSxxx, 主控为IL0373
{
  public:
    GxEPD2_213_EXT3(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_213c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
};

// https://www.pervasivedisplays.com/product/2-66-e-ink-displays/, 其他同上
// 需深度魔改GxEPD2库, GxEPD2_290c.h的WIDTH改为152
// GxEPD2_290c.cpp的_InitDisplay()函数的最后三行改为
// _writeData (WIDTH);
// _writeData (HEIGHT >> 8);
// _writeData (HEIGHT & 0xff);
class GxEPD2_266_EXT3 : public GxEPD2_290c // 型号为xE2266CSxxx, 主控为IL0373
{
  public:
    GxEPD2_266_EXT3(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_290c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
};

#endif // _GxEPD2_Extra_H_
