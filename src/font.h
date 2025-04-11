#ifndef __FONT_H__
#define __FONT_H__

#include <stdbool.h>
#include <u8g2_fonts.h>

extern const uint8_t u8g2_font_helvB14_tf[] U8G2_FONT_SECTION("u8g2_font_helvB14_tf");
extern const uint8_t u8g2_font_helvB18_tf[] U8G2_FONT_SECTION("u8g2_font_helvB18_tf");
extern const uint8_t u8g2_font_fub30_tf[] U8G2_FONT_SECTION("u8g2_font_fub30_tf");
// 新版 u8g2 库已内置文泉驿字体
#if USE_FULL_GB2313_FONT == true
extern const uint8_t u8g2_font_wqy12_t_gb2312b[] U8G2_FONT_SECTION("u8g2_font_wqy12_t_gb2312b");
extern const uint8_t u8g2_font_wqy14_t_gb2312b[] U8G2_FONT_SECTION("u8g2_font_wqy14_t_gb2312b");
extern const uint8_t u8g2_font_wqy16_t_gb2312b[] U8G2_FONT_SECTION("u8g2_font_wqy16_t_gb2312b");
#define u8g2_font_wqy12_t u8g2_font_wqy12_t_gb2312b
#define u8g2_font_wqy14_t u8g2_font_wqy14_t_gb2312b
#define u8g2_font_wqy16_t u8g2_font_wqy16_t_gb2312b
#else
extern const uint8_t u8g2_font_wqy12_t_gb2312[] U8G2_FONT_SECTION("u8g2_font_wqy12_t_gb2312");
extern const uint8_t u8g2_font_wqy14_t_gb2312[] U8G2_FONT_SECTION("u8g2_font_wqy14_t_gb2312");
extern const uint8_t u8g2_font_wqy16_t_gb2312[] U8G2_FONT_SECTION("u8g2_font_wqy16_t_gb2312");
#define u8g2_font_wqy12_t u8g2_font_wqy12_t_gb2312
#define u8g2_font_wqy14_t u8g2_font_wqy14_t_gb2312
#define u8g2_font_wqy16_t u8g2_font_wqy16_t_gb2312
#endif
extern const uint8_t u8g2_font_qweather_icon_16[] U8G2_FONT_SECTION("u8g2_font_qweather_icon_16");
extern const uint8_t u8g2_font_bili_icon_16[] U8G2_FONT_SECTION("u8g2_font_bili_icon_16");

#endif // __FONT_H__
