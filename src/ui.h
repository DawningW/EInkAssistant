#ifndef __UI_H__
#define __UI_H__

#include "font.h"
#include "draw.h"

#define COLOR_PRIMARY GxEPD_BLACK
#define COLOR_SECONDARY (EPD_DRIVER::hasColor ? GxEPD_RED : GxEPD_BLACK)
#define COLOR_ERROR (EPD_DRIVER::hasColor ? GxEPD_RED : GxEPD_BLACK)

enum class UISize {
    XS, SM, MD, LG, XL, CUSTOM
};

template<int WIDTH, int HEIGHT>
struct UITraits {
    static constexpr UISize value =
        ((int) UI_RESOLUTION > -1) ? (UISize) UI_RESOLUTION :
        (WIDTH <= 200 && HEIGHT <= 200) ? UISize::XS :
        (WIDTH >= 640 && HEIGHT >= 480) ? UISize::XL :
        (WIDTH >= 400 && HEIGHT >= 300) ? UISize::LG :
        (WIDTH >= 300 && HEIGHT >= 200) ? UISize::MD :
        UISize::SM;
};

#define UI_IMPL_FUNCS { \
public: \
    static void loading(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void smartConfig(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void syncTimeFailed(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void lowPower(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void update(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void titleBar(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery); \
    static void weather(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery); \
    static void bilibili(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2); \
    static void display(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip, const String &text); \
    static void about(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip); \
}

template<UISize size>
class UIImpl {};
template<>
class UIImpl<UISize::XS> UI_IMPL_FUNCS;
template<>
class UIImpl<UISize::SM> UI_IMPL_FUNCS;
template<>
class UIImpl<UISize::MD> UI_IMPL_FUNCS;
template<>
class UIImpl<UISize::LG> UI_IMPL_FUNCS;
template<>
class UIImpl<UISize::XL> UI_IMPL_FUNCS;
template<>
class UIImpl<UISize::CUSTOM> UI_IMPL_FUNCS;

#define UI UIImpl<UITraits<EPD_DRIVER::WIDTH, EPD_DRIVER::HEIGHT>::value>

#endif // __UI_H__
