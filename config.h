#ifndef __CONFIG_H__
#define __CONFIG_H__

// 选择使用 SMPL_2021.7.22_V2 开发板
// #include "config_SMPL.h"
// 选择使用 OurEDA 设计的 CoreBoard_ESP32 开发板
// #include "config_CoreBoard.h"

// 型号, 用于检查更新, 如果你使用其他开发板请修改型号, 避免被小程序推送更新
#ifndef MODEL
#define MODEL ""
#endif
// 版本号
#define VERSION "V0.0.1"
// 版本代码, 用于检查更新
#define VERSION_CODE 1

/****************************** 硬件配置 ******************************/
// 可选值参考 https://github.com/ZinggJM/GxEPD2/blob/master/examples/GxEPD2_Example/GxEPD2_display_selection_new_style.h
// 墨水屏种类
#define EPD_TYPE GxEPD2_3C
// 墨水屏驱动
#define EPD_DRIVER GxEPD2_213c

#ifndef HAS_CONFIG
// 墨水屏引脚
#define EPD_CS -1
#define EPD_DC -1
#define EPD_RST -1
#define EPD_BUSY -1
#if !defined(ESP8266) // ESP8266 不可设置 SPI 引脚
#define EPD_CLK -1
#define EPD_MOSI -1
#endif
// 墨水屏旋转方向, 0~3
#define EPD_ROTATION 3
// 切换界面按键引脚
#define KEY_SWITCH -1
// 按键引脚上下拉模式, INPUT/INPUT_PULLUP/INPUT_PULLDOWN
#define KEY_PIN_MODE INPUT
// 按键引脚触发方式, LOW/HIGH
#define KEY_TRIGGER_LEVEL LOW
// 硬件是否支持深睡 (例如 ESP8266 需要将 gpio16 与 rst 相连才可从深睡中唤醒)
#define SUPPORT_DEEP_SLEEP true
#endif

#if (EPD_CS == -1) || (EPD_DC == -1) || (EPD_RST == -1) || (EPD_BUSY == -1) || (!defined(ESP8266) && (EPD_CLK == -1 || EPD_MOSI == -1)) || (KEY_SWITCH == -1)
#warning "请先在 config.h 中配置硬件引脚"
#error "Please configure hardware pins first in config.h"
#endif

/****************************** 软件配置 ******************************/
// 是否启用开屏加载界面
#define ENABLE_LOADING_SCREEN true
// 是否启用电量显示
#define ENABLE_BATTERY_DISPLAY false
// 电量低于多少百分比时进入休眠模式
#define BATTERY_LOW_PERCENTAGE 5
// 指定秒数无操作后进入休眠模式, 0 为不休眠
#if SUPPORT_DEEP_SLEEP
#define SLEEP_TIMEOUT 180
#else
#define SLEEP_TIMEOUT 0
#endif
// 夜晚暂停更新或光线较暗时暂停更新
#define SLEEP_ON_NIGHT false
// 时区, 可取值请参考 TZ.h 或 https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#if defined(ESP8266)
#include <TZ.h>
#define TIMEZONE TZ_Asia_Shanghai
#elif defined(ESP32)
#define TIMEZONE "CST-8"
#endif
// NTP服务器地址, 最多可填写三个, 用逗号隔开, 服务器列表请见 https://dns.icoa.cn/ntp/
#define NTP_SERVERS "time.pool.aliyun.com", "pool.ntp.org"
// 配网二维码内容, 可以换成你自己的链接或小程序
#define WIFI_CONFIG_URL "http://wx.ai-thinker.com/api/old/wifi/config"
// 和风天气 API KEY
#define QWEATHER_KEY "your_qweather_key"

// 恭喜你, 已经完成了所有配置, 其余配置可通过小程序或 HTTP 接口修改, 详见 README.md

#endif // __CONFIG_H__
