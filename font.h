#ifndef __FONT_H__
#define __FONT_H__

#include <stdbool.h>
#include <u8g2_fonts.h>

extern const uint8_t u8g2_font_helvB14_tf[] U8G2_FONT_SECTION("u8g2_font_helvB14_tf");
extern const uint8_t u8g2_font_fub30_tf[] U8G2_FONT_SECTION("u8g2_font_fub30_tf");
extern const uint8_t u8g2_font_wqy12_t_gb2312b[] U8G2_FONT_SECTION("u8g2_font_wqy12_t_gb2312b");
#define u8g2_font_wqy12_t u8g2_font_wqy12_t_gb2312b // 新版 u8g2 库已内置此字体
extern const uint8_t u8g2_font_qweather_icon_16[] U8G2_FONT_SECTION("u8g2_font_qweather_icon_16");
extern const uint8_t u8g2_font_bili_icon_16[] U8G2_FONT_SECTION("u8g2_font_bili_icon_16");

#endif // __FONT_H__
