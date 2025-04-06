#ifndef __LANG_H__
#define __LANG_H__

const char TEXT_LOADING[] = "正在初始化, 请稍候...";
const char TEXT_LOW_POWER[] = "电量不足, 请充电";
const char TEXT_SMART_CONFIG[] = "请用微信扫描二维码配置网络";
const char TEXT_TIME_ERROR[] = "无法同步时间, 请检查网络连接!";
const char TEXT_UPDATING_1[] = "正在升级中, 请不要断开电源";
const char TEXT_UPDATING_2[] = "升级完成后将自动重启";

const char WEEKDAYS[][7] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
const char FORMAT_DATETIME[] = "%%Y年%%m月%%d日 %s %%H:%%M";
const char TEXT_WEATHER_FAILED[] = "获取天气数据失败!";

const char TEXT_CUSTOM_EMPTY[] = "请访问%s/display来设置文本";

#endif // __LANG_H__
