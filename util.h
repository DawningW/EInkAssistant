#ifndef __UTIL_H__
#define __UTIL_H__

#include <time.h>
#include <Arduino.h>

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

uint32_t calculateCRC32(const uint8_t *data, size_t length);
String datetimeToString(const char *fmt, tm *ptime);
String humanizeNumber(uint64_t number);
String humanizeByte(size_t bytes);

#endif // __UTIL_H__
