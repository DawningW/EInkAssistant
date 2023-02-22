/**
 * E-Ink Assistant 墨水屏智能助理
 *
 * 基于 ESP8266 使用 Arduino 开发的低功耗墨水屏应用, 具有时钟, 日历, 天气等功能, 且提供接口可自行扩展
 *
 * @author QingChenW
 */

#include "config.h"

#include <functional>
#include <vector>
#include <TZ.h>
#include <time.h>
#include <user_interface.h>
#include <coredecls.h>
#include <smartconfig.h>
#include <Arduino.h>
#include <Ticker.h>
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include "font.h"
#include "bitmap.h"
#include "lang.h"
#include "draw.h"
#include "API.hpp"
#include "util.h"

#define MIME_TYPE(t) (mime::mimeTable[mime::type::t].mimeType)

// 参数为是否首次刷新
typedef std::function<void(bool)> DrawPageFunc;

const char *product_name = "einkassistant";
const char *model_name = MODEL;
const char *version = VERSION;
const uint32_t version_code = VERSION_CODE;

Ticker keyDebounce;
EPD_CLASS epd(EPD_DRIVER(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
ESP8266WebServer server(80);
std::vector<DrawPageFunc> pages;
bool keyPressed;
time_t sleepTimer;
uint8_t pageCustom;
String displayBuffer;

struct Config {
    uint8_t version;          // 配置版本, 若与程序版本号不同则重置配置
    char hostname[25];        // 主机名
    uint32_t update_interval; // 首页更新间隔(秒)
    int8_t theme;             // 图标样式, -1: 跟随时间, 0: 白色描边(白天), 1: 黑色填充(夜间)
    uint8_t hour_step;        // 逐小时天气预报的显示步长(时)
    uint32_t location_id;     // 位置id
    uint32_t bilibili_uid;    // B站uid
    char bilibili_cookie[36]; // B站cookie(SESSDATA)
} config;

struct RTCData {
    uint32_t crc32;     // TODO 是否有必要为 RTC memory 添加 CRC 校验
    time_t wakeup_time; // 从睡眠中唤醒时的时间(秒)
    int8_t page;        // 当前正在显示的页面
    time_t next_update; // 下次更新时间(秒)
    char date[11];      // 当日天气预报的缓存
    char sunrise[6];
    char sunset[6];
    char moonPhase[10];
    uint16_t moonPhaseIcon;
} rtcdata;

int8_t getBatteryLevel() {
#if ENABLE_BATTERY_DISPLAY
    #error "请实现 int8_t getBatteryLevel() 函数"
#else
    return -1;
#endif
}

bool resetConfig(bool wifi = false) {
    if (wifi) {
        struct station_config conf;
        *conf.ssid = 0;
        *conf.password = 0;
        ETS_UART_INTR_DISABLE();
        wifi_station_set_config(&conf);
        ETS_UART_INTR_ENABLE();
    }
    config.version = version_code;
    strcpy(config.hostname, product_name);
    config.update_interval = 3600;
    config.theme = -1;
    config.hour_step = 1;
    config.location_id = 101010100;
    config.bilibili_uid = 1;
    strcpy(config.bilibili_cookie, "");
    EEPROM.put(0, config);
    return EEPROM.commit();
}

// unit: second
void gotoSleep(uint32_t s) {
    if (Serial) Serial.printf("Sleep for %d seconds\n", s); // 这里不能把字符串存在 Flash 里, 否则墨水屏颜色会变淡
    pinMode(KEY_SWITCH, INPUT);
    // epd.hibernate();
    delay(1000);
    // 从深度睡眠中唤醒也会导致 RTC timer 被清零, 所以存在 RTC memory 里
    rtcdata.wakeup_time = time(nullptr) + s;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcdata, sizeof(RTCData));
    ESP.deepSleep(s * 1000000UL, WAKE_RF_DEFAULT);
}

// auto calculate sleep time by update interval
void gotoSleep() {
    time_t sleep_time = rtcdata.next_update - time(nullptr);
    if (sleep_time <= 60) {
        // 要休眠的时间小于一分钟, 可能是还没到点就刷新了
        sleep_time = config.update_interval;
    }
    time_t max_time = ESP.deepSleepMax() / 1000000;
    gotoSleep(min(sleep_time, max_time));
}

void checkBattery() {
    if ((uint8_t) getBatteryLevel() <= BATTERY_LOW_PERCENTAGE) {
        startDraw(epd);
        u8g2Fonts.setFont(u8g2_font_wqy12_t);
        drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() * 3 / 4, TEXT_LOW_POWER);
        endDraw(epd);
        gotoSleep(0);
    }
}

void prepareOTA() {
    WiFiUDP::stopAll();
    startDraw(epd);
    epd.fillTriangle(epd.width() / 2, 8, epd.width() / 2 - 24, 32, epd.width() / 2 + 24, 32, GxEPD_BLACK);
    epd.fillRect(epd.width() / 2 - 12, 32, 24, 42, GxEPD_BLACK);
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() - 16, TEXT_UPDATING_1);
    drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() - 4, TEXT_UPDATING_2);
    endDraw(epd);
}

bool isNight(String time) {
    if (config.theme >= 0) return config.theme;
    uint8_t hour = time.substring(11, 13).toInt();
    return hour < 6 || hour >= 18;
}

void drawTitleBar(const char *title, bool sleeping, int8_t rssi, int8_t battery) {
    // Draw title
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    u8g2Fonts.drawUTF8(2, 12, title);
    // Draw battery level
    uint16_t x = epd.width() - 2;
    if (battery >= 0) {
        epd.drawRect(x - 21, 2, 19, 10, GxEPD_BLACK);
        epd.fillRect(x - 2, 4, 2, 6, GxEPD_BLACK);
        if (battery > 0)
            epd.fillRect(x - 19, 4, 15 * battery / 100, 6, GxEPD_BLACK);
        x -= 25;
    }
    // Draw WiFi RSSI
    epd.fillRect(x - 11, 8, 3, 4, GxEPD_BLACK);
    if (rssi >= -80)
        epd.fillRect(x - 7, 5, 3, 7, GxEPD_BLACK);
    if (rssi >= -70)
        epd.fillRect(x - 3, 2, 3, 10, GxEPD_BLACK);
    if (rssi == 31) { // Failure, invalid value.
        int16_t x1 = x - 11, y1 = 2;
        int16_t x2 = x, y2 = 12;
        epd.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
        epd.drawLine(x2, y1, x1, y2, GxEPD_BLACK);
    }
    x -= 15;
    // Draw sleep status
    if (sleeping) {
        u8g2Fonts.drawUTF8(x - 6, 12, "Z");
    }
    // Draw separator
    epd.drawFastHLine(0, 15, epd.width(), GxEPD_BLACK);
}

void drawWeatherNow(Weather &weather) {
    // Draw temperature and humidity
    u8g2Fonts.setFont(u8g2_font_helvB14_tf);
    u8g2Fonts.setCursor(0, 36);
    u8g2Fonts.printf("%d°C/%d%%", weather.temp, weather.humidity);
    // Draw weathervane and wind speed
    uint16_t x = 0;
    if (weather.wind360 >= 0) {
        drawArrow(epd, x + 8, 47, -18, weather.wind360, 8, 16);
        x += 18;
    }
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    u8g2Fonts.setCursor(x, 52);
    u8g2Fonts.printf("%s %d级", weather.windDir.c_str(), weather.windScale);
    // Draw Weather icon and text
    u8g2Fonts.setFont(u8g2_font_qweather_icon_16);
    drawCenteredString(u8g2Fonts, epd.width() / 2, 40, getWeatherIcon(weather.icon, isNight(weather.time)));
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    drawCenteredString(u8g2Fonts, epd.width() / 2, 52, weather.text.c_str());
}

void drawForecastHourly(HourlyForecast &forecast) {
    // Draw grid
    uint16_t grid_w = epd.width() / forecast.length;
    epd.drawFastHLine(0, 57, epd.width(), GxEPD_BLACK);
    for (uint8_t i = 1; i < forecast.length; i++) {
        epd.drawFastVLine(grid_w * i, 58, epd.height() - 58, GxEPD_BLACK);
    }
    // Draw hourly forecast
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    for (uint8_t i = 0; i < forecast.length; i++) {
        Weather &weather = forecast.weather[i];
        uint16_t x = grid_w * i + grid_w / 2;
        drawCenteredString(u8g2Fonts, x, 69, weather.time.substring(11, 16).c_str());
        u8g2Fonts.setFont(u8g2_font_qweather_icon_16);
        drawCenteredString(u8g2Fonts, x, epd.height() - 12, getWeatherIcon(weather.icon, isNight(weather.time)));
        u8g2Fonts.setFont(u8g2_font_wqy12_t);
        String temp = String(weather.temp) + "°C";
        drawCenteredString(u8g2Fonts, x, epd.height() - 2, temp.c_str());
    }
}

void drawForecastDaily(DailyForecast &forecast) {
    DailyWeather &weather = *forecast.weather;
    // Draw sunrise, sunset and moon phase
    u8g2Fonts.setFont(u8g2_font_wqy12_t);
    u8g2Fonts.setCursor(130, 28);
    u8g2Fonts.printf("日出%s", weather.sunrise.c_str());
    u8g2Fonts.setCursor(130, 41);
    u8g2Fonts.printf("日落%s", weather.sunset.c_str());
    drawCenteredString(u8g2Fonts, 154, 54, weather.moonPhase.c_str());
    // Draw moon icon
    u8g2Fonts.setFont(u8g2_font_qweather_icon_16);
    const char *icon = getWeatherIcon(weather.moonPhaseIcon);
    u8g2Fonts.drawUTF8(epd.width() - u8g2Fonts.getUTF8Width(icon) - 4, 48, icon);
}

uint8_t addPage(DrawPageFunc callback) {
    pages.push_back(callback);
    return pages.size() - 1;
}

void refreshPage() {
    if (Update.isRunning()) return;
    Serial.print(F("Refresh page "));
    Serial.println(rtcdata.page);
    pages[rtcdata.page](false);
}

void showPage(int8_t page) {
    if (Update.isRunning()) return;
    if (page == rtcdata.page) {
        refreshPage();
        return;
    }
    Serial.print(F("Show page "));
    Serial.println(page);
    pages[page](true);
    rtcdata.page = page;
}

void lastPage() {
    int8_t page = rtcdata.page - 1;
    if (page < 0) page = pages.size() - 1;
    showPage(page);
}

void nextPage() {
    int8_t page = rtcdata.page + 1;
    page %= pages.size();
    showPage(page);
}

IRAM_ATTR void onKeyPressed() {
    keyDebounce.once_ms(20, []() {
        if (digitalRead(KEY_SWITCH) == LOW) {
            keyPressed = true;
        }
    });
}

void initPages() {
    // 主页面: 天气
    addPage([](bool init) {
        time_t timestamp = time(nullptr);
        tm *ptime = localtime(&timestamp);
#if SLEEP_ON_NIGHT == true
        if (ptime->tm_hour >= 0 && ptime->tm_hour < 5) {
            goto update_timer;
        }
#endif
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
        if (dailyWeather.date.substring(8, 10).toInt() != ptime->tm_mday) {
            success &= api.getForecastDaily(dailyForecast, config.location_id);
            strcpy(rtcdata.date, dailyWeather.date.c_str());
            strcpy(rtcdata.sunrise, dailyWeather.sunrise.c_str());
            strcpy(rtcdata.sunset, dailyWeather.sunset.c_str());
            strcpy(rtcdata.moonPhase, dailyWeather.moonPhase.c_str());
            rtcdata.moonPhaseIcon = dailyWeather.moonPhaseIcon;
        }

        startDraw(epd);
        bool isSleeping = SLEEP_TIMEOUT && ESP.getResetInfoPtr()->reason == REASON_DEEP_SLEEP_AWAKE;
        drawTitleBar(title.c_str(), isSleeping, WiFi.RSSI(), getBatteryLevel());
        drawWeatherNow(currentWeather);
        drawForecastHourly(hourlyForecast);
        drawForecastDaily(dailyForecast);
        if (!success) {
            u8g2Fonts.setFont(u8g2_font_wqy12_t);
            u8g2Fonts.setForegroundColor(GxEPD_RED);
            drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() / 2 + 6, TEXT_WEATHER_FAILED);
            u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        }
        endDraw(epd);

        sleepTimer = millis();
update_timer:
        rtcdata.next_update = (time(nullptr) / config.update_interval + 1) * config.update_interval;
        // 如果上一次更新失败导致 next_update 没有更新, 从而导致这次更新时 next_update 仍在当前时间之前
        if (rtcdata.next_update < time(nullptr)) {
            rtcdata.next_update += config.update_interval;
        }
    });
    // Bilibili页面: 显示up主粉丝数, 播放量和点赞量
    addPage([](bool init) {
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
        u8g2Fonts.setFont(u8g2_font_fub30_tf);
        drawCenteredString(u8g2Fonts, (x + epd.width() - 8) / 2, 56, humanizeNumber(bilibili.follower).c_str());
        if (success) {
            u8g2Fonts.setFont(u8g2_font_bili_icon_16);
            u8g2Fonts.drawUTF8(x + 10, epd.height() - 16, "\uE6E3");
            u8g2Fonts.drawUTF8(x + 68, epd.height() - 16, "\uE6E0");
            u8g2Fonts.setFont(u8g2_font_wqy12_t);
            u8g2Fonts.drawUTF8(x + 34, epd.height() - 20, humanizeNumber(bilibili.view).c_str());
            u8g2Fonts.drawUTF8(x + 94, epd.height() - 20, humanizeNumber(bilibili.likes).c_str());
        }
        endDraw(epd);
    });
    // 下位机页面: 显示从 HTTP 接口发来的文本
    pageCustom = addPage([](bool init) {
        startDraw(epd);
        u8g2Fonts.setFont(u8g2_font_wqy12_t);
        u8g2Fonts.setCursor(0, 12);
        if (displayBuffer.length()) {
            u8g2Fonts.print(displayBuffer);
        } else {
            u8g2Fonts.printf(TEXT_CUSTOM_EMPTY, WiFi.localIP().toString().c_str());
        }
        endDraw(epd);
    });
    // 关于页面: 显示IP, 版本和版权信息以及必不可少的一言
    addPage([](bool init) {
        Hitokoto hitokoto = {};
        api.getHitokoto(hitokoto);

        startDraw(epd);
        u8g2Fonts.setFont(u8g2_font_wqy12_t);
        u8g2Fonts.setCursor(0, 24);
        u8g2Fonts.printf("  墨水屏智能助理 %s\n", version);
        u8g2Fonts.println("  Copyright (C) 2022-2023 WC");
        u8g2Fonts.println("  气象数据由 和风天气 提供");
        u8g2Fonts.println("");
        u8g2Fonts.println("  IP: " + WiFi.localIP().toString());
        u8g2Fonts.println("");
        u8g2Fonts.print("  ");
        u8g2Fonts.println(hitokoto.sentence);
        endDraw(epd);
    });
}

// TODO 适配硬件 RTC, 我这块板子上没有, 等换板子再说吧
void setup() {
    // BUG 上电的时候有概率墨水屏颜色会变淡, 还不知道是什么原因造成的
    /*
#if defined(ESP8266) && EPD_CS == 15 // ESP8266 的 GPIO15 在上电时为低电平, 会导致墨水屏被选中
    digitalWrite(EPD_CS, HIGH);
    pinMode(EPD_CS, OUTPUT);
    delay(500);
#endif
    */
    u8g2Fonts.begin(epd);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    epd.setRotation(EPD_ROTATION);

    checkBattery();

    Serial.begin(115200);
    Serial.println();
    Serial.print(F("E-Ink Assistant, version: "));
    Serial.println(version);
    Serial.println(F("Made by QingChenW with love"));

    uint32_t resetReason = ESP.getResetInfoPtr()->reason;
    // 这里不判断 SLEEP_TIMEOUT 是因为我要的效果是只要从休眠中唤醒就不显示加载界面
    if (resetReason != REASON_DEEP_SLEEP_AWAKE) {
#if !ENABLE_LOADING_SCREEN
        epd.init(0);
        epd.clearScreen();
        epd.hibernate();
#else
        startDraw(epd);
        u8g2Fonts.setFont(u8g2_font_wqy12_t);
        drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() * 3 / 4, TEXT_LOADING);
        endDraw(epd, true);
#endif
    } else {
        ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcdata, sizeof(RTCData));
        // 从 RTC memory 里取出唤醒时的时间, 并判断是否需要继续休眠
        timeval tv;
        tv.tv_sec = rtcdata.wakeup_time;
        settimeofday(&tv, nullptr);
        if (rtcdata.page == 0 && rtcdata.next_update - rtcdata.wakeup_time > 60) {
            gotoSleep();
        }
    }
    EEPROM.begin(sizeof(Config));
    EEPROM.get(0, config);
    if (config.version != version_code) {
        Serial.printf_P(PSTR("Update config from %d to %d\n"), config.version, version_code);
        resetConfig();
    }
    if (resetReason != REASON_DEEP_SLEEP_AWAKE)
        delay(3000); // 如果去掉延时, 建议不要显示加载界面

    volatile bool has_set_time = false;
    settimeofday_cb([&has_set_time](bool sntp) { has_set_time = true; });
    configTime(TIMEZONE, NTP_SERVERS);
    Serial.print(F("WiFi connecting"));
    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();
    while (true) {
        wl_status_t status = WiFi.status();
        uint8_t retry = 0;
        while (status != WL_CONNECTED && retry++ < 20) {
            delay(1000);
            Serial.print('.');
            status = WiFi.status();
        }
        Serial.println();
        if (status == WL_CONNECTED) {
            Serial.print(F("Connected, IP address: "));
            Serial.println(WiFi.localIP());
            break;
        } else {
            if (SLEEP_TIMEOUT && resetReason == REASON_DEEP_SLEEP_AWAKE) {
                Serial.println(F("Failed, try connect next time"));
                gotoSleep();
            }
            Serial.println(F("Failed, try SmartConfig"));
            startDraw(epd);
            drawQRCode(epd, (epd.width() - 58) / 2, 16, 2, WIFI_CONFIG_URL);
            u8g2Fonts.setFont(u8g2_font_wqy12_t);
            drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() - 12, TEXT_SMART_CONFIG);
            endDraw(epd, true);
            smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);
            WiFi.beginSmartConfig();
            while (!WiFi.smartConfigDone()) {
                delay(1000);
                Serial.print('.');
            }
        }
    }
    WiFi.setHostname(config.hostname);

    initPages();
    if (!SLEEP_TIMEOUT || resetReason != REASON_DEEP_SLEEP_AWAKE) {
        server.on("/", HTTP_GET, []() {
            server.send(200, MIME_TYPE(html), F("Hello World!"));
        });
        server.on("/version", HTTP_GET, []() {
            StaticJsonDocument<256> doc;
            doc["product"] = product_name;
            doc["model"] = model_name;
            doc["version"] = version;
            doc["version_code"] = version_code;
            doc["sdk_version"] = ESP.getFullVersion();
            String str;
            serializeJson(doc, str);
            server.send(200, MIME_TYPE(json), str);
        });
        server.on("/status", HTTP_GET, []() {
            StaticJsonDocument<256> doc;
            doc["reset_reason"] = ESP.getResetReason();
            doc["free_heap"] = ESP.getFreeHeap();
            doc["heap_fragment"] = ESP.getHeapFragmentation();
            doc["max_free_block"] = ESP.getMaxFreeBlockSize();
            doc["RSSI"] = WiFi.RSSI();
            doc["page"] = rtcdata.page;
            doc["next_update"] = rtcdata.next_update;
            String str;
            serializeJson(doc, str);
            server.send(200, MIME_TYPE(json), str);
        });
        server.on("/config", HTTP_GET, []() {
            StaticJsonDocument<256> doc;
            doc["ip"] = WiFi.localIP().toString();
            doc["mask"] = WiFi.subnetMask().toString();
            doc["gateway"] = WiFi.gatewayIP().toString();
            doc["hostname"] = config.hostname;
            doc["update_itv"] = config.update_interval;
            doc["theme"] = config.theme;
            doc["hour_step"] = config.hour_step;
            doc["locid"] = config.location_id;
            doc["uid"] = config.bilibili_uid;
            String str;
            serializeJson(doc, str);
            server.send(200, MIME_TYPE(json), str);
        });
        server.on("/config", HTTP_POST, []() {
            String value = server.arg("hostname");
            if (value.length() > 0)
                strncpy(config.hostname, value.c_str(), 24);
            value = server.arg("update_itv");
            if (value.length() > 0)
                config.update_interval = max((int32_t) value.toInt(), 300);
            value = server.arg("theme");
            if (value.length() > 0)
                config.theme = min(max((int32_t) value.toInt(), -1), 1);
            value = server.arg("hour_step");
            if (value.length() > 0)
                config.hour_step = max((int32_t) value.toInt(), 1);
            value = server.arg("locid");
            if (value.length() > 0)
                config.location_id = value.toInt();
            value = server.arg("uid");
            if (value.length() > 0)
                config.bilibili_uid = value.toInt();
            value = server.arg("bili_cookie");
            if (value.length() > 0)
                strncpy(config.bilibili_cookie, value.c_str(), 36);
            EEPROM.put(0, config);
            refreshPage();
            server.send(200, MIME_TYPE(txt), EEPROM.commit() ? "OK" : "FAIL");
        });
        server.on("/reset", HTTP_GET, []() {
            bool wifi = server.arg("wifi") == "true";
            server.send(200, MIME_TYPE(txt), resetConfig(wifi) ? "OK" : "FAIL");
        });
        server.on("/update", HTTP_POST, []() {
            server.sendHeader("Connection", "close");
            server.send(200, MIME_TYPE(txt), !Update.hasError() ? "OK" : "FAIL");
            delay(1000); // 不延时的话无法返回响应
            Serial.println(F("Rebooting..."));
            ESP.restart();
        }, []() {
            HTTPUpload &upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.print(F("Upload file: "));
                Serial.println(upload.filename.c_str());
                prepareOTA();
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if (Update.begin(maxSketchSpace)) {
                    Serial.println(F("Start to update"));
                } else {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    Serial.print(F("Update Success: "));
                    Serial.println(upload.totalSize);
                } else {
                    Update.printError(Serial);
                }
            }
            yield();
        });
        server.on("/display", HTTP_POST, []() {
            displayBuffer = server.arg("content");
            if (rtcdata.page == pageCustom) {
                refreshPage();
            }
            delay(3000);
            server.send(200, MIME_TYPE(txt), "OK");
        });
        server.begin();
        MDNS.begin(config.hostname);
        MDNS.addService("http", "tcp", 80);
    }
    Serial.print(F("Waiting for time sync..."));
    sleepTimer = millis();
    while (!has_set_time) {
        delay(500);
        if (millis() - sleepTimer > 30000) {
            Serial.println(F("NTP timeout"));
            Serial.print(F("Try to use http api..."));
            uint64_t timestamp;
            if (api.getTimestamp(timestamp)) {
                timeval tv = {
                    .tv_sec = timestamp / 1000LL,
                    .tv_usec = (timestamp % 1000L) * 1000L
                };
                settimeofday_cb(BoolCB());
                settimeofday(&tv, NULL);
                break;
            }
            if (!SLEEP_TIMEOUT || resetReason != REASON_DEEP_SLEEP_AWAKE) {
                startDraw(epd);
                u8g2Fonts.setFont(u8g2_font_wqy12_t);
                u8g2Fonts.setForegroundColor(GxEPD_RED);
                drawCenteredString(u8g2Fonts, epd.width() / 2, epd.height() * 3 / 4, TEXT_TIME_ERROR);
                u8g2Fonts.setForegroundColor(GxEPD_BLACK);
                endDraw(epd, true);
            }
            gotoSleep();
        }
    }
    settimeofday_cb(BoolCB());
    Serial.println(F("Done"));
    // XXX 这段以及 gotoSleep() 的逻辑给我写蒙了, 不知道有没有 bug
    if (SLEEP_TIMEOUT && resetReason == REASON_DEEP_SLEEP_AWAKE && rtcdata.page == 0) {
        if (time(nullptr) - rtcdata.next_update > -60) {
            while (time(nullptr) - rtcdata.next_update < 0)
                delay(1000);
            refreshPage();
        }
        gotoSleep();
    }
    refreshPage();

    pinMode(KEY_SWITCH, INPUT_PULLUP);
    attachInterrupt(KEY_SWITCH, onKeyPressed, FALLING);
}

void loop() {
    checkBattery();

    MDNS.update();
    server.handleClient();
    if (Update.isRunning()) return;

    if (keyPressed) {
        while (digitalRead(KEY_SWITCH) == LOW)
            delay(10);
        nextPage();
        keyPressed = false;
    }

    if (rtcdata.page == 0) {
#if SLEEP_TIMEOUT > 0
        if (millis() - sleepTimer >= SLEEP_TIMEOUT * 1000) {
            time_t timestamp = time(nullptr);
            tm *time = localtime(&timestamp);
            String title = datetimeToString(FORMAT_DATETIME, time);
            startDraw(epd);
            drawTitleBar(title.c_str(), true, WiFi.RSSI(), getBatteryLevel());
            epd.displayWindow(0, 0, epd.width(), 16);
            epd.hibernate();
            gotoSleep();
        }
#endif
        time_t delta = time(nullptr) - rtcdata.next_update;
        if (delta > 0 && delta <= 60) {
            refreshPage();
        }
    }

    // TODO 在这里延时 100ms 以上可以让 MCU 进入 Modem-sleep 模式
    // 可以将待机电流降低到 20mA, 但是 http 响应时间会增加 1 秒
    // delay(100);
}
