#ifndef __DRAW_H__
#define __DRAW_H__

#include "config.h"

// 启用或禁用GxEPD2_GFX基类, 可用于将引用或指针作为参数传递到显示实例, 额外占用~1.2k代码
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <qrcode.h>

#include "GxEPD2_Extra.h"

typedef EPD_TYPE<EPD_DRIVER, EPD_DRIVER::HEIGHT> EPD_CLASS;

void startDraw(EPD_CLASS &epd, int32_t bgcolor = GxEPD_WHITE);
void endDraw(EPD_CLASS &epd, bool partial_update = false);
void drawCenteredString(U8G2_FOR_ADAFRUIT_GFX &u8g2, uint16_t x, uint16_t y, const char *str);
void drawArrow(EPD_CLASS &epd, uint16_t x, uint16_t y, int16_t asize, float aangle, uint16_t pwidth, uint16_t plength);
void drawQRCode(EPD_CLASS &epd, uint16_t x, uint16_t y, uint8_t scale, const char *text, uint8_t version = 3, uint8_t ecc = ECC_LOW);

const char *getWeatherIcon(uint16_t id, bool fill = false);

#endif // __DRAW_H__
