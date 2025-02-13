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

// 来源 老王 2.9元 2.13寸价签
// 丝印为WFT0213CZ16LW
// 对应型号应为微雪 2.13inch e-Paper HAT (D)
// 主控为IL0373
// 需要按3色屏幕使用
class GxEPD2_213_Z16 :public GxEPD2_213c
{
  public:
    using GxEPD2_213c::GxEPD2_213c;
    void clearScreen(uint8_t black_value, uint8_t color_value) 
    {
      GxEPD2_213c::clearScreen(color_value, ~black_value);
    }

    void writeScreenBuffer(uint8_t black_value, uint8_t color_value) 
    {
      GxEPD2_213c::writeScreenBuffer(color_value, ~black_value);
    }

    void writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) 
    {
      GxEPD2_213c::writeImage(color, black, x, y, w, h, !invert, mirror_y, pgm);
    }

    void writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) 
    {
      GxEPD2_213c::writeImagePart(color, black, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, !invert, mirror_y, pgm);
    }

    void writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false)
    {
      GxEPD2_213c::writeNative(data2, data1, x, y, w, h, !invert, mirror_y, pgm);
    }

    void drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false)
    {
      GxEPD2_213c::drawImage(color, black, x, y, w, h, !invert, mirror_y, pgm);
    }

    void drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                      int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) 
    {
      GxEPD2_213c::drawImagePart(color, black, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, !invert, mirror_y, pgm);
    }

    void drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) 
    {
      GxEPD2_213c::drawNative(data2, data1, x, y, w, h, !invert, mirror_y, pgm);
    }

    void clearScreen(uint8_t value = 0xFF) override
    {
      clearScreen(value, value);
    }

    void writeScreenBuffer(uint8_t value = 0xFF) override
    {
      writeScreenBuffer(value, value);
    }

    void writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) override
    {
      writeImage(bitmap, bitmap, x, y, w, h, invert, mirror_y, pgm);
    }

    void writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false) override
    {
      writeImagePart(bitmap, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
    }

    void drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false)
    {
      drawImage(bitmap, bitmap, x, y, w, h, invert, mirror_y, pgm);
    }

    void drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                      int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false)
    {
      drawImagePart(bitmap, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
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
