/**
 * E-Ink Assistant 墨水屏智能助理
 *
 * 基于 ESP8266/ESP32 使用 Arduino 开发的低功耗墨水屏应用, 具有时钟, 日历, 天气等功能, 且提供接口可自行扩展
 *
 * @author QingChenW
 */

#include "main.h"

#include <functional>
#include <vector>
#include <time.h>
#if defined(ESP8266)
#include <user_interface.h>
#include <coredecls.h>
#include <smartconfig.h>
#elif defined(ESP32)
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_sntp.h>
#endif
#include <Arduino.h>
#include <Ticker.h>
#if defined(ESP8266)
#include <Updater.h>
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#define RTC_DATA_ATTR
typedef ESP8266WebServer WebServer;
#elif defined(ESP32)
#include <Update.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <detail/mimetable.h>
#endif
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "draw.h"
#include "ui.h"
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
WebServer server(80);
API<> api;
std::vector<DrawPageFunc> pages;
bool keyPressed;
time_t sleepTimer;
uint8_t pageCustom;
String displayBuffer;

Config config;
RTC_DATA_ATTR RTCData rtcdata;

int8_t getBatteryLevel() {
#if ENABLE_BATTERY_DISPLAY
    #error "请实现 int8_t getBatteryLevel() 函数"
#else
    return -1;
#endif
}

bool resetConfig(bool wifi = false) {
    if (wifi) {
#if defined(ESP8266)
        struct station_config conf;
        memset(&conf, 0, sizeof(conf));
        ETS_UART_INTR_DISABLE();
        wifi_station_set_config(&conf);
        ETS_UART_INTR_ENABLE();
#elif defined(ESP32)
        wifi_config_t conf;
        memset(&conf, 0, sizeof(conf));
        esp_wifi_set_config(WIFI_IF_STA, &conf);
#endif
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
#if defined(ESP8266)
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcdata, sizeof(RTCData));
    ESP.deepSleep(s * 1000000UL, WAKE_RF_DEFAULT);
#elif defined(ESP32)
    if (s > 0)
        esp_sleep_enable_timer_wakeup(s * 1000000UL);
    esp_deep_sleep_start();
#endif
}

// auto calculate sleep time by update interval
void gotoSleep() {
    time_t sleep_time = rtcdata.next_update - time(nullptr);
    if (sleep_time <= 60) {
        // 要休眠的时间小于一分钟, 可能是还没到点就刷新了
        sleep_time = config.update_interval;
    }
#if defined(ESP8266)
    time_t max_time = ESP.deepSleepMax() / 1000000;
    gotoSleep(min(sleep_time, max_time));
#elif defined(ESP32)
    gotoSleep(sleep_time);
#endif
}

void checkBattery() {
    if ((uint8_t) getBatteryLevel() <= BATTERY_LOW_PERCENTAGE) {
        UI::lowPower(epd, u8g2Fonts);
        gotoSleep(0);
    }
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
        if (digitalRead(KEY_SWITCH) == KEY_TRIGGER_LEVEL) {
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
        bool isSleeping = false;
#if SLEEP_TIMEOUT > 0
        if (millis() - sleepTimer >= SLEEP_TIMEOUT * 1000) {
            isSleeping = true;
        } else {
#if defined(ESP8266)
            isSleeping = ESP.getResetInfoPtr()->reason == REASON_DEEP_SLEEP_AWAKE;
#elif defined(ESP32)
            isSleeping = esp_reset_reason() == ESP_RST_DEEPSLEEP;
#endif
        }
#endif

        UI::weather(epd, u8g2Fonts, ptime, isSleeping, WiFi.RSSI(), getBatteryLevel());

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
        UI::bilibili(epd, u8g2Fonts);
    });
    // 下位机页面: 显示从 HTTP 接口发来的文本
    pageCustom = addPage([](bool init) {
        UI::display(epd, u8g2Fonts, WiFi.localIP().toString(), displayBuffer);
    });
    // 关于页面: 显示IP, 版本和版权信息以及必不可少的一言
    addPage([](bool init) {
        UI::about(epd, u8g2Fonts, WiFi.localIP().toString());
    });
}

// TODO 适配硬件 RTC, 我这块板子上没有, 等换板子再说吧
// TODO 适配温湿度传感器
void setup() {
    /*
#if defined(ESP8266) && EPD_CS == 15 // ESP8266 的 GPIO15 在上电时为低电平, 会导致墨水屏被选中
    digitalWrite(EPD_CS, HIGH);
    pinMode(EPD_CS, OUTPUT);
    delay(500);
#endif
    */
#ifdef ESP32
    SPI.begin(EPD_CLK, -1, EPD_MOSI, EPD_CS);
#endif
    u8g2Fonts.begin(epd);
    u8g2Fonts.setForegroundColor(COLOR_PRIMARY);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    epd.setRotation(EPD_ROTATION);

    checkBattery();

    Serial.begin(115200);
    Serial.println();
    Serial.print(F("E-Ink Assistant, version: "));
    Serial.println(version);
    Serial.println(F("Made by QingChenW with love"));

#if defined(ESP8266)
#define RST_REASON_DEEP_SLEEP REASON_DEEP_SLEEP_AWAKE
    uint32_t resetReason = ESP.getResetInfoPtr()->reason;
#elif defined(ESP32)
#define RST_REASON_DEEP_SLEEP ESP_RST_DEEPSLEEP
    uint32_t resetReason = esp_reset_reason();
#endif
    // 这里不判断 SLEEP_TIMEOUT 是因为我要的效果是只要从休眠中唤醒就不显示加载界面
    if (resetReason != RST_REASON_DEEP_SLEEP) {
#if !ENABLE_LOADING_SCREEN
        epd.init(0);
        epd.clearScreen();
        epd.hibernate();
#else
        UI::loading(epd, u8g2Fonts);
#endif
    } else {
#ifdef ESP8266
        ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcdata, sizeof(RTCData));
#endif
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
    if (resetReason != RST_REASON_DEEP_SLEEP)
        delay(3000); // 如果去掉延时, 建议不要显示加载界面

#if defined(ESP8266)
    volatile bool has_set_time = false;
    settimeofday_cb([&has_set_time](bool sntp) { has_set_time = true; });
    configTime(TIMEZONE, NTP_SERVERS);
#elif defined(ESP32)
    static volatile bool has_set_time = false;
    sntp_set_time_sync_notification_cb([](struct timeval *tv) { has_set_time = true; });
    configTzTime(TIMEZONE, NTP_SERVERS);
#endif
    Serial.print(F("WiFi connecting"));
    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin(
#ifdef WIFI_SSID
        WIFI_SSID
#endif
#ifdef WIFI_PASSWORD
        , WIFI_PASSWORD
#endif
    );
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
            if (SLEEP_TIMEOUT && resetReason == RST_REASON_DEEP_SLEEP) {
                Serial.println(F("Failed, try connect next time"));
                gotoSleep();
            }
            Serial.println(F("Failed, try SmartConfig"));
            UI::smartConfig(epd, u8g2Fonts);
#if defined(ESP8266)
            smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);
            WiFi.beginSmartConfig();
#elif defined(ESP32)
            WiFi.beginSmartConfig(SC_TYPE_ESPTOUCH_AIRKISS);
#endif
            while (!WiFi.smartConfigDone()) {
                delay(1000);
                Serial.print('.');
            }
        }
    }
    WiFi.setHostname(config.hostname);

    initPages();
    if (!SLEEP_TIMEOUT || resetReason != RST_REASON_DEEP_SLEEP) {
        server.on("/", HTTP_GET, []() {
            server.send(200, MIME_TYPE(html), F("Hello World!"));
        });
        server.on("/version", HTTP_GET, []() {
            StaticJsonDocument<256> doc;
            doc["product"] = product_name;
            doc["model"] = model_name;
            doc["version"] = version;
            doc["version_code"] = version_code;
#if defined(ESP8266)
            doc["sdk_version"] = ESP.getFullVersion();
#elif defined(ESP32)
            doc["sdk_version"] = ESP.getSdkVersion();
#endif
            String str;
            serializeJson(doc, str);
            server.send(200, MIME_TYPE(json), str);
        });
        server.on("/status", HTTP_GET, []() {
            StaticJsonDocument<256> doc;
#if defined(ESP8266)
            doc["reset_reason"] = ESP.getResetReason();
            doc["free_heap"] = ESP.getFreeHeap();
            doc["heap_fragment"] = ESP.getHeapFragmentation();
            doc["max_free_block"] = ESP.getMaxFreeBlockSize();
#elif defined(ESP32)
            doc["reset_reason"] = esp_reset_reason();
            doc["free_heap"] = ESP.getFreeHeap();
            doc["heap_max_free_block"] = ESP.getMaxAllocHeap();
            doc["free_psram"] = ESP.getFreePsram();
            doc["psram_max_free_block"] = ESP.getMaxAllocPsram();
#endif
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
                config.update_interval = max((int) value.toInt(), 300);
            value = server.arg("theme");
            if (value.length() > 0)
                config.theme = min(max((int) value.toInt(), -1), 1);
            value = server.arg("hour_step");
            if (value.length() > 0)
                config.hour_step = max((int) value.toInt(), 1);
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
#ifdef ESP8266
                WiFiUDP::stopAll();
#endif
                UI::update(epd, u8g2Fonts);
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
        server.on("/shipmode", HTTP_POST, []() {
            startDraw(epd);
            endDraw(epd);
            server.send(200, MIME_TYPE(txt), "OK");
            gotoSleep(0);
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
            break;
        }
    }
#if defined(ESP8266)
    settimeofday_cb(BoolCB());
#elif defined(ESP32)
    sntp_set_time_sync_notification_cb(nullptr);
#endif
    if (!has_set_time) {
        Serial.println(F("NTP timeout"));
        Serial.print(F("Try to use http api..."));
        uint64_t timestamp;
        if (api.getTimestamp(timestamp)) {
            timeval tv = {
                .tv_sec = timestamp / 1000LL,
                .tv_usec = (timestamp % 1000L) * 1000L
            };
            settimeofday(&tv, NULL);
        } else {
            if (!SLEEP_TIMEOUT || resetReason != RST_REASON_DEEP_SLEEP) {
                UI::syncTimeFailed(epd, u8g2Fonts);
            }
            gotoSleep();
        }
    }
    Serial.println(F("Done"));
    if (SLEEP_TIMEOUT && resetReason == RST_REASON_DEEP_SLEEP && rtcdata.page == 0) {
        if (time(nullptr) - rtcdata.next_update > -60) {
            while (time(nullptr) - rtcdata.next_update < 0)
                delay(1000);
            refreshPage();
        }
        gotoSleep();
    }
    refreshPage();

    pinMode(KEY_SWITCH, KEY_PIN_MODE);
    attachInterrupt(KEY_SWITCH, onKeyPressed, KEY_TRIGGER_LEVEL == LOW ? FALLING : RISING);
}

void loop() {
    checkBattery();

#ifdef ESP8266
    MDNS.update();
#endif
    server.handleClient();
    if (Update.isRunning()) return;

    if (keyPressed) {
        while (digitalRead(KEY_SWITCH) == KEY_TRIGGER_LEVEL)
            delay(10);
        nextPage();
        keyPressed = false;
    }

    if (rtcdata.page == 0) {
#if SLEEP_TIMEOUT > 0
        if (millis() - sleepTimer >= SLEEP_TIMEOUT * 1000) {
            if (EPD_DRIVER::hasPartialUpdate) {
                time_t timestamp = time(nullptr);
                tm *time = localtime(&timestamp);
                UI::titleBar(epd, u8g2Fonts, time, true, WiFi.RSSI(), getBatteryLevel());
            } else {
                refreshPage();
            }
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
