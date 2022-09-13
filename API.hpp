/**
 * 将所有的 API 请求放到一个文件中方便在其他项目中复用, 此文件以 MIT 协议开源
 * 
 * @author QingChenW
 * @copyright MIT license
 */

#ifndef __API_HPP__
#define __API_HPP__

#include <Arduino.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// 以下参数请按照您的项目需求修改
#include "config.h"
#ifndef QWEATHER_KEY
#define QWEATHER_KEY ""
#endif

struct Weather {
    String time;
    int8_t temp;
    int8_t humidity;
    int16_t wind360;
    String windDir;
    int8_t windScale;
    uint8_t windSpeed;
    uint16_t icon;
    String text;
};

struct DailyWeather {
    String date;
    String sunrise;
    String sunset;
    String moonPhase;
    uint16_t moonPhaseIcon;
    int8_t tempMax;
    int8_t tempMin;
    int8_t humidity;
    uint16_t iconDay;
    String textDay;
    uint16_t iconNight;
    String textNight;
    int16_t wind360Day;
    String windDirDay;
    int8_t windScaleDay;
    uint8_t windSpeedDay;
    int16_t wind360Night;
    String windDirNight;
    int8_t windScaleNight;
    uint8_t windSpeedNight;
};

struct HourlyForecast {
    Weather *weather;
    uint8_t length;
    uint8_t interval;
};

struct DailyForecast {
    DailyWeather *weather;
    uint8_t length;
};

struct Hitokoto {
    String sentence;
    String from;
    String from_who;
};

struct Bilibili {
    uint64_t follower;
    uint64_t view;
    uint64_t likes;
};

template <uint8_t MAX_RETRY = 3>
class API {
    using callback = std::function<bool(JsonDocument&)>;
    using precall = std::function<void()>;

private:
    BearSSL::WiFiClientSecure client;
    HTTPClient http;

    /**
     * @brief 向指定的 Restful API 发送请求并自动将返回的数据解析为 json 对象, 如果失败则重试最多 MAX_RETRY 次
     * 
     * @param url Restful API 地址
     * @param cb 数据解析完成回调, 返回值代表是否成功解析
     * @param pre 请求发送前拦截器, 可以用于添加 Cookie 或 Token 等
     * @return 是否成功请求 
     */
    bool getRestfulAPI(String url, callback cb, precall pre = precall()) {
        Serial.print(F("Request "));
        Serial.println(url);
        DynamicJsonDocument doc(8192);
        for (uint8_t i = 0; i < MAX_RETRY; i++) {
            bool shouldRetry = false;
            if (http.begin(client, url)) {
                if (pre) pre();
                int httpCode = http.GET();
                if (httpCode == 200) {
                    DeserializationError error = deserializeJson(doc, client);
                    if (!error) {
                        http.end();
                        return cb(doc);
                    } else {
                        Serial.print(F("Parse JSON failed, error: "));
                        Serial.println(error.f_str());
                        shouldRetry = error == DeserializationError::IncompleteInput;
                    }
                } else {
                    Serial.print(F("Get failed, error: "));
                    if (httpCode < 0) {
                        Serial.println(http.errorToString(httpCode));
                        shouldRetry = httpCode == HTTPC_ERROR_CONNECTION_FAILED ||
                                        httpCode == HTTPC_ERROR_CONNECTION_LOST ||
                                        httpCode == HTTPC_ERROR_READ_TIMEOUT;
                    } else {
                        Serial.println(httpCode);
                    }
                }
                http.end();
            } else {
                Serial.println(F("Unable to connect"));
            }
            if (!shouldRetry) break;
            Serial.println(F("Retry after 30 second"));
            delay(30000);
        }
        return false;
    }

public:
    API() {
        // client.setFingerprint(fingerprint);
        // 不安全就不安全吧, 主要是我比较懒(
        client.setInsecure();
        // 默认Buffer大小是16KB+512B......
        client.setBufferSizes(4096, 1024);
        // 默认超时时间是5000ms, 如果觉得不够长就取消下面的注释
        // http.setTimeout(10000);
    }

    ~API() {}

    // 获取 WiFiClient
    BearSSL::WiFiClientSecure& wifiClient() { return client; }

    // 获取 HTTPClient
    HTTPClient& httpClient() { return http; }

    // 和风天气 - 实时天气: https://dev.qweather.com/docs/api/weather/weather-now/
    bool getWeatherNow(Weather &result, uint32_t locid) {
        return getRestfulAPI("https://devapi.qweather.com/v7/weather/now?gzip=n&key=" QWEATHER_KEY "&location=" + String(locid), [&result](JsonDocument& json) {
            if (strcmp(json["code"], "200") != 0) {
                Serial.print(F("Get weather failed, error: "));
                Serial.println(json["code"].as<const char*>());
                return false;
            }
            JsonObject now = json["now"];
            result.time = now["obsTime"].as<const char*>();
            result.temp = atoi(now["temp"]);
            result.humidity = atoi(now["humidity"]);
            result.wind360 = atoi(now["wind360"]);
            result.windDir = now["windDir"].as<const char*>();
            result.windScale = atoi(now["windScale"]);
            result.windSpeed = atoi(now["windSpeed"]);
            result.icon = atoi(now["icon"]);
            result.text = now["text"].as<const char*>();
            return true;
        });
    }

    // 和风天气 - 逐小时天气预报: https://dev.qweather.com/docs/api/weather/weather-hourly-forecast/
    bool getForecastHourly(HourlyForecast &result, uint32_t locid) {
        return getRestfulAPI("https://devapi.qweather.com/v7/weather/24h?gzip=n&key=" QWEATHER_KEY "&location=" + String(locid), [&result](JsonDocument& json) {
            if (strcmp(json["code"], "200") != 0) {
                Serial.print(F("Get hourly forecast failed, error: "));
                Serial.println(json["code"].as<const char*>());
                return false;
            }
            uint8_t i, hours = json["hourly"].size();
            for (i = 0; i < result.length; i++) {
                if (i * result.interval >= hours) break;
                Weather &weather = result.weather[i];
                JsonObject hourly = json["hourly"][i * result.interval];
                weather.time = hourly["fxTime"].as<const char*>();
                weather.temp = atoi(hourly["temp"]);
                weather.humidity = atoi(hourly["humidity"]);
                weather.wind360 = atoi(hourly["wind360"]);
                weather.windDir = hourly["windDir"].as<const char*>();
                weather.windScale = atoi(hourly["windScale"]);
                weather.windSpeed = atoi(hourly["windSpeed"]);
                weather.icon = atoi(hourly["icon"]);
                weather.text = hourly["text"].as<const char*>();
            }
            result.length = i;
            return true;
        });
    }

    // 和风天气 - 逐天天气预报: https://dev.qweather.com/docs/api/weather/weather-daily-forecast/
    bool getForecastDaily(DailyForecast &result, uint32_t locid) {
        return getRestfulAPI("https://devapi.qweather.com/v7/weather/3d?gzip=n&key=" QWEATHER_KEY "&location=" + String(locid), [&result](JsonDocument& json) {
            if (strcmp(json["code"], "200") != 0) {
                Serial.print(F("Get daily forecast failed, error: "));
                Serial.println(json["code"].as<const char*>());
                return false;
            }
            uint8_t i;
            for (i = 0; i < result.length; i++) {
                DailyWeather &weather = result.weather[i];
                JsonObject daily = json["daily"][i];
                weather.date = daily["fxDate"].as<const char*>();
                weather.sunrise = daily["sunrise"].as<const char*>();
                weather.sunset = daily["sunset"].as<const char*>();
                weather.moonPhase = daily["moonPhase"].as<const char*>();
                weather.moonPhaseIcon = atoi(daily["moonPhaseIcon"]);
                weather.tempMax = atoi(daily["tempMax"]);
                weather.tempMin = atoi(daily["tempMin"]);
                weather.humidity = atoi(daily["humidity"]);
                weather.iconDay = atoi(daily["iconDay"]);
                weather.textDay = daily["textDay"].as<const char*>();
                weather.iconNight = atoi(daily["iconNight"]);
                weather.textNight = daily["textNight"].as<const char*>();
                weather.wind360Day = atoi(daily["wind360Day"]);
                weather.windDirDay = daily["windDirDay"].as<const char*>();
                weather.windScaleDay = atoi(daily["windScaleDay"]);
                weather.windSpeedDay = atoi(daily["windSpeedDay"]);
                weather.wind360Night = atoi(daily["wind360Night"]);
                weather.windDirNight = daily["windDirNight"].as<const char*>();
                weather.windScaleNight = atoi(daily["windScaleNight"]);
                weather.windSpeedNight = atoi(daily["windSpeedNight"]);
            }
            result.length = i;
            return true;
        });
    }

    // 一言: https://developer.hitokoto.cn/sentence/
    bool getHitokoto(Hitokoto &result) {
        return getRestfulAPI("https://v1.hitokoto.cn/?max_length=15", [&result](JsonDocument& json) {
            result.sentence = json["hitokoto"].as<const char*>();
            result.from = json["from"].as<const char*>();
            result.from_who = json["from_who"].as<const char*>();
            return true;
        });
    }

    // B站粉丝
    bool getFollower(Bilibili &result, uint32_t uid) {
        return getRestfulAPI("https://api.bilibili.com/x/relation/stat?vmid=" + String(uid), [&result](JsonDocument& json) {
            if (json["code"] != 0) {
                Serial.print(F("Get bilibili follower failed, error: "));
                Serial.println(json["message"].as<const char*>());
                return false;
            }
            result.follower = json["data"]["follower"];
            return true;
        });
    }

    // B站总播放量和点赞数
    bool getLikes(Bilibili &result, uint32_t uid, const char *cookie) {
        return getRestfulAPI("https://api.bilibili.com/x/space/upstat?mid=" + String(uid), [&result](JsonDocument& json) {
            if (json["code"] != 0) {
                Serial.print(F("Get bilibili likes failed, error: "));
                Serial.println(json["message"].as<const char*>());
                return false;
            }
            result.view = json["data"]["archive"]["view"];
            result.likes = json["data"]["likes"];
            return true;
        }, [this, &cookie]() {
            http.addHeader("Cookie", String("SESSDATA=") + cookie + ";");
        });
    }
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_API)
API<> api;
#endif

#endif // __API_HPP__
