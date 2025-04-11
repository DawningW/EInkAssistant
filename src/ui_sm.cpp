#include "ui.h"

#include "main.h"
#include "lang.h"
#include "bitmap.h"
#include "util.h"

template <>
void UIImpl<UISize::SM>::loading(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 3 / 4, TEXT_LOADING);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::SM>::smartConfig(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    drawQRCode(epd, (epd.width() - 58) / 2, 16, 2, WIFI_CONFIG_URL);
    u8g2.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 12, TEXT_SMART_CONFIG);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::SM>::syncTimeFailed(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.setForegroundColor(COLOR_ERROR);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 3 / 4, TEXT_TIME_ERROR);
    u8g2.setForegroundColor(GxEPD_BLACK);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::SM>::lowPower(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 3 / 4, TEXT_LOW_POWER);
    endDraw(epd);
}

template <>
void UIImpl<UISize::SM>::update(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    epd.fillTriangle(epd.width() / 2, 8, epd.width() / 2 - 24, 32, epd.width() / 2 + 24, 32, GxEPD_BLACK);
    epd.fillRect(epd.width() / 2 - 12, 32, 24, 42, GxEPD_BLACK);
    u8g2.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 16, TEXT_UPDATING_1);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 4, TEXT_UPDATING_2);
    endDraw(epd);
}

template <>
void UIImpl<UISize::SM>::titleBar(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    String title = datetimeToString(FORMAT_DATETIME, ptime);
    startDraw(epd);
    drawTitleBar(epd, u8g2, title.c_str(), sleeping, rssi, battery);
    epd.displayWindow(0, 0, epd.width(), 16);
    epd.hibernate();
}

static void drawWeatherNow(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, Weather &weather) {
    // Draw temperature and humidity
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.setCursor(0, 36);
    u8g2.printf("%d°C/%d%%", weather.temp, weather.humidity);
    // Draw weathervane and wind speed
    uint16_t x = 0;
    if (weather.wind360 >= 0) {
        drawArrow(epd, x + 8, 47, -18, weather.wind360, 8, 16);
        x += 18;
    }
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.setCursor(x, 52);
    u8g2.printf("%s %s级", weather.windDir.c_str(), weather.windScale.c_str());
    // Draw weather icon and text
    u8g2.setFont(u8g2_font_qweather_icon_16);
    drawCenteredString(u8g2, epd.width() / 2, 40, getWeatherIcon(weather.icon, isNight(weather.time)));
    u8g2.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2, epd.width() / 2, 52, weather.text.c_str());
}

static void drawForecastHourly(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, HourlyForecast &forecast) {
    // Draw grid
    uint16_t grid_w = epd.width() / forecast.length;
    epd.drawFastHLine(0, 57, epd.width(), GxEPD_BLACK);
    for (uint8_t i = 1; i < forecast.length; i++) {
        epd.drawFastVLine(grid_w * i, 58, epd.height() - 58, GxEPD_BLACK);
    }
    // Draw hourly forecast
    u8g2.setFont(u8g2_font_wqy12_t);
    for (uint8_t i = 0; i < forecast.length; i++) {
        Weather &weather = forecast.weather[i];
        uint16_t x = grid_w * i + grid_w / 2;
        drawCenteredString(u8g2, x, 69, weather.time.substring(11, 16).c_str());
        u8g2.setFont(u8g2_font_qweather_icon_16);
        drawCenteredString(u8g2, x, epd.height() - 12, getWeatherIcon(weather.icon, isNight(weather.time)));
        u8g2.setFont(u8g2_font_wqy12_t);
        String temp = String(weather.temp) + "°C";
        drawCenteredString(u8g2, x, epd.height() - 2, temp.c_str());
    }
}

static void drawForecastDaily(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, DailyForecast &forecast) {
    DailyWeather &weather = *forecast.weather;
    // Draw sunrise, sunset and moon phase
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.setCursor(epd.width() - 84, 28);
    u8g2.printf("日出%s", weather.sunrise.c_str());
    u8g2.setCursor(epd.width() - 84, 41);
    u8g2.printf("日落%s", weather.sunset.c_str());
    drawCenteredString(u8g2, epd.width() - 60, 54, weather.moonPhase.c_str());
    // Draw moon icon
    u8g2.setFont(u8g2_font_qweather_icon_16);
    const char *icon = getWeatherIcon(weather.moonPhaseIcon);
    u8g2.drawUTF8(epd.width() - u8g2.getUTF8Width(icon) - 4, 48, icon);
}

template <>
void UIImpl<UISize::SM>::weather(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    String title = datetimeToString(FORMAT_DATETIME, ptime);
    Weather currentWeather = {};
    Weather hourlyWeather[6] = {};
    DailyWeather dailyWeather = {};
    dailyWeather.date = rtcdata.date;
    dailyWeather.sunrise = rtcdata.sunrise;
    dailyWeather.sunset = rtcdata.sunset;
    dailyWeather.moonPhase = rtcdata.moonPhase;
    dailyWeather.moonPhaseIcon = rtcdata.moonPhaseIcon;
    HourlyForecast hourlyForecast = {
        .weather = hourlyWeather,
        .length = ARRAY_LENGTH(hourlyWeather),
        .interval = config.hour_step
    };
    DailyForecast dailyForecast = {
        .weather = &dailyWeather,
        .length = 1
    };
    bool success = api.getWeatherNow(currentWeather, config.location_id)
                    & api.getForecastHourly(hourlyForecast, config.location_id);
    if (rtcdata.location_id != config.location_id ||
            dailyWeather.date.substring(8, 10).toInt() != ptime->tm_mday) {
        success &= api.getForecastDaily(dailyForecast, config.location_id);
        strcpy(rtcdata.date, dailyWeather.date.c_str());
        strcpy(rtcdata.sunrise, dailyWeather.sunrise.c_str());
        strcpy(rtcdata.sunset, dailyWeather.sunset.c_str());
        strcpy(rtcdata.moonPhase, dailyWeather.moonPhase.c_str());
        rtcdata.moonPhaseIcon = dailyWeather.moonPhaseIcon;
    }
    rtcdata.location_id = config.location_id;

    startDraw(epd);
    drawTitleBar(epd, u8g2, title.c_str(), sleeping, rssi, battery);
    drawWeatherNow(epd, u8g2, currentWeather);
    drawForecastHourly(epd, u8g2, hourlyForecast);
    drawForecastDaily(epd, u8g2, dailyForecast);
    if (!success) {
        u8g2.setFont(u8g2_font_wqy12_t);
        u8g2.setForegroundColor(COLOR_ERROR);
        drawCenteredString(u8g2, epd.width() / 2, epd.height() / 2 + 6, TEXT_WEATHER_FAILED);
        u8g2.setForegroundColor(GxEPD_BLACK);
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::SM>::bilibili(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    Bilibili bilibili = {};
    api.getFollower(bilibili, config.bilibili_uid);
    bool success = false;
    if (*config.bilibili_cookie != '\0') {
        success = api.getLikes(bilibili, config.bilibili_uid, config.bilibili_cookie);
    }

    startDraw(epd);
    // epd.drawInvertedBitmap(8, (epd.height() - 55) / 2, IMG_BILI_LOGO, 52, 55, GxEPD_BLACK);
    epd.drawInvertedBitmap(8, (epd.height() - 65) / 2, IMG_BILI_LOGO_2, 68, 65, GxEPD_BLACK);
    uint16_t x = 80;
    u8g2.setFont(u8g2_font_fub30_tf);
    drawCenteredString(u8g2, (x + epd.width() - 8) / 2, 56, humanizeNumber(bilibili.follower).c_str());
    if (success) {
        u8g2.setFont(u8g2_font_bili_icon_16);
        u8g2.drawUTF8(x + 10, epd.height() - 16, "\uE6E3");
        u8g2.drawUTF8(x + 68, epd.height() - 16, "\uE6E0");
        u8g2.setFont(u8g2_font_wqy12_t);
        u8g2.drawUTF8(x + 34, epd.height() - 20, humanizeNumber(bilibili.view).c_str());
        u8g2.drawUTF8(x + 94, epd.height() - 20, humanizeNumber(bilibili.likes).c_str());
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::SM>::display(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip, const String &text) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.setCursor(0, 12);
    if (text.length() > 0) {
        u8g2.print(text);
    } else {
        u8g2.printf(TEXT_CUSTOM_EMPTY, ip.c_str());
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::SM>::about(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip) {
    Hitokoto hitokoto = {};
    api.getHitokoto(hitokoto);

    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.setCursor(0, 24);
    u8g2.printf("  墨水屏智能助理 %s\n", version);
    u8g2.println("  Copyright (C) 2022-2025 WC");
    u8g2.println("  气象数据由 和风天气 提供");
    u8g2.println("");
    u8g2.println("  IP: " + ip);
    u8g2.println("");
    u8g2.print("  ");
    u8g2.println(hitokoto.sentence);
    endDraw(epd);
}
