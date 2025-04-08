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

// 来源 老王 2.13寸价签
// 屏幕同微雪2.13inch e-Paper HAT (B) V3/2.13inch e-Paper Module (C)
// 虽然丝印与微雪的一模一样, 但应该是定制版, 空闲时为低电平, 且只支持黑白
// 由于这块屏幕需使用内置的LUT, 但GxEPD2的IL0373单色屏驱动没有一个是用内置LUT的, 只有GxEPD2_213c是, 且与微雪官方例程几乎完全一致
// 所以决定基于GxEPD2_213c魔改, 但GxEPD2_213c是三色屏幕, 虽然都使用IL0373, 但改动较大
// 1. 将_InitDisplay()和_Init_Part()函数中写寄存器0x50的值改为0x97 (表示白色边框, 如需黑色边框改为0x57)
// 2. 驱动单色屏时0x13命令才表示发送黑色数据, 因此需修改clearScreen, writeScreenBuffer, writeImage, writeImagePart函数
//    注释掉原发送0x13命令和数据的代码, 将0x10改为0x13
// 这样修改后可以使用GxEPD2_BW, 无需使用GxEPD2_3C, 更省内存
class GxEPD2_213_Z16 : public GxEPD2_213c // 型号为WFT0213CZ16, 主控为IL0373
{
  public:
    GxEPD2_213_Z16(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_213c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
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

// 来源 老王 2.9寸价签
// 屏幕同微雪2.9inch e-Paper Module (B) V3/2.9inch e-Paper Module (C)
// 和老王的2.13寸价签同理, 基于GxEPD2_290c魔改
// 1. 将_PowerOff()函数中写寄存器0x50的值改为0x17
// 2. 将_InitDisplay()和_Init_Part()函数中写寄存器0x50的值改为0x97 (表示白色边框, 如需黑色边框改为0x57)
// 3. 驱动单色屏时0x13命令才表示发送黑色数据, 因此需修改clearScreen, writeScreenBuffer, writeImage, writeImagePart函数
//    注释掉原发送0x13命令和数据的代码, 将0x10改为0x13
// 这样修改后可以使用GxEPD2_BW, 无需使用GxEPD2_3C, 更省内存
class GxEPD2_290_Z10 : public GxEPD2_290c // 型号为WFT0290CZ10, 主控为IL0373
{
  public:
    GxEPD2_290_Z10(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_290c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
};

// 来自https://www.pervasivedisplays.com/product/2-66-e-ink-displays/, 其他同GxEPD2_213_EXT3
// 需深度魔改GxEPD2库, GxEPD2_290c.h的WIDTH改为152
// GxEPD2_290c.cpp的_InitDisplay()函数的最后三行改为
// _writeData (WIDTH);
// _writeData (HEIGHT >> 8);
// _writeData (HEIGHT & 0xff);
class GxEPD2_266_EXT3 : public GxEPD2_290c // 型号为xE2266CSxxx, 主控为IL0373, QE2266HS051也用这个
{
  public:
    GxEPD2_266_EXT3(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_290c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
};

// 来源 老王 4.2寸价签
// 屏幕同微雪4.2inch e-Paper Module (B) V2
// 基于GxEPD2_420c魔改, 该屏幕为三色屏, 仅BUSY引脚电平相反, 其他无需修改
class GxEPD2_420c_Z15 : public GxEPD2_420c // 型号为WFT0420CZ15, 主控为IL0398
{
  public:
    GxEPD2_420c_Z15(int16_t cs, int16_t dc, int16_t rst, int16_t busy) : GxEPD2_420c(cs, dc, rst, busy)
    {
      this->_busy_level = HIGH;
    }
};

#endif // _GxEPD2_Extra_H_
