#include "util.h"

extern const char *WEEKDAYS[];

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xffffffff;
    while (length--) {
        uint8_t c = *data++;
        for (uint32_t i = 0x80; i > 0; i >>= 1) {
            bool bit = crc & 0x80000000;
            if (c & i) {
                bit = !bit;
            }
            crc <<= 1;
            if (bit) {
                crc ^= 0x04c11db7;
            }
        }
    }
    return crc;
}

String datetimeToString(const char *fmt, tm *ptime) {
    char fmt2[32], str[36];
    snprintf(fmt2, sizeof(fmt2), fmt, WEEKDAYS[ptime->tm_wday]);
    strftime(str, sizeof(str), fmt2, ptime);
    return str;
}

String humanizeNumber(uint64_t number) {
    String str;
    if (number >= 10000) {
        str = String(number / 10000.0, 1U) + "W";
    } else {
        str = String(number);
    }
    return str;
}

String humanizeByte(size_t bytes) {
    String str;
    if (bytes >= 1073741824) {
        str = String(bytes / 1073741824.0) + "GB";
    } else if (bytes >= 1048576) {
        str = String(bytes / 1048576.0) + "MB";
    } else if (bytes >= 1024) {
        str = String(bytes / 1024.0) + "KB";
    } else {
        str = String(bytes) + "B";
    }
    return str;
}
