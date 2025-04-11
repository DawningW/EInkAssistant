#include "ui.h"

#include "main.h"
#include "lang.h"
#include "bitmap.h"
#include "util.h"

template <>
void UIImpl<UISize::XL>::loading(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::loading(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::smartConfig(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::smartConfig(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::syncTimeFailed(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::syncTimeFailed(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::lowPower(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::lowPower(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::update(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::update(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::titleBar(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    UIImpl<UISize::LG>::titleBar(epd, u8g2, ptime, sleeping, rssi, battery);
}

template <>
void UIImpl<UISize::XL>::weather(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    UIImpl<UISize::LG>::weather(epd, u8g2, ptime, sleeping, rssi, battery);
}

template <>
void UIImpl<UISize::XL>::bilibili(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    UIImpl<UISize::LG>::bilibili(epd, u8g2);
}

template <>
void UIImpl<UISize::XL>::display(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip, const String &text) {
    UIImpl<UISize::LG>::display(epd, u8g2, ip, text);
}

template <>
void UIImpl<UISize::XL>::about(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip) {
    UIImpl<UISize::LG>::about(epd, u8g2, ip);
}
