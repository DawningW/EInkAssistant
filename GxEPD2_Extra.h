#ifndef _GxEPD2_Extra_H_
#define _GxEPD2_Extra_H_

#include <epd/GxEPD2_290.h>
#include <epd3c/GxEPD2_213c.h>
#include <epd3c/GxEPD2_290c.h>

// https://www.e-paper-display.cn/products_detail/productId=376.html, 已停产
// GxEPD2_290.h的WIDTH改为200, HEIGHT改为200
/*
修改GxEPD2_290.cpp的LUTDefault_full数组为
0x32,
0x66, 0x66, 0x44, 0x66, 0xAA, 0x11, 0x80, 0x08, 0x11, 0x18, 0x81, 0x18, 0x11, 0x88, 0x11,
0x88, 0x11, 0x88, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x5F, 0xAF, 0xFF, 0xFF, 0x2F, 0x00
修改GxEPD2_290.cpp的LUTDefault_part数组为
0x32,
0x10, 0x18, 0x18, 0x28, 0x18, 0x18, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x11, 0x22, 0x63, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00
 */
class GxEPD2_154_D27 : public GxEPD2_290 // 型号为HINK-E0154A05(GDEH0154D27), 主控为SSD1608(IL3820)
{
  public:
    using GxEPD2_290::GxEPD2_290;
};

// GxEPD2_213_B72.h的WIDTH_VISIBLE改为104, HEIGHT改为212
// 该屏幕不支持局部刷新, 因此还需将hasPartialUpdate和hasFastPartialUpdate改为false
class GxEPD2_213_A07 : public GxEPD2_213_B72 // 型号为HINK-E0213A07, 主控为SSD1675A(IL3897)
{
  public:
    using GxEPD2_213_B72::GxEPD2_213_B72;
};

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
