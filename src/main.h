#ifndef __MAIN_H__
#define __MAIN_H__

#include "config.h"

#include <stdint.h>
#include <stdbool.h>
#define NO_GLOBAL_API
#include "API.hpp"

struct Config {
    uint8_t version;          // 配置版本, 若与程序版本号不同则重置配置
    char hostname[25];        // 主机名
    uint32_t update_interval; // 首页更新间隔(秒)
    int8_t theme;             // 图标样式, -1: 跟随时间, 0: 白色描边(白天), 1: 黑色填充(夜间)
    uint8_t hour_step;        // 逐小时天气预报的显示步长(时)
    uint32_t location_id;     // 位置id
    uint32_t bilibili_uid;    // B站uid
    char bilibili_cookie[36]; // B站cookie(SESSDATA)
};

struct RTCData {
    uint32_t crc32;       // TODO 是否有必要为 RTC memory 添加 CRC 校验
    time_t wakeup_time;   // 从睡眠中唤醒时的时间(秒)
    int8_t page;          // 当前正在显示的页面
    time_t next_update;   // 下次更新时间(秒)
    uint32_t location_id; // 当日天气预报的缓存
    char date[11];
    char sunrise[6];
    char sunset[6];
    char moonPhase[10];
    uint16_t moonPhaseIcon;
};

extern const char *product_name;
extern const char *model_name;
extern const char *version;
extern const uint32_t version_code;

extern API<> api;
extern Config config;
extern RTCData rtcdata;

#endif // __MAIN_H__
