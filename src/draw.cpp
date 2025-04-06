#include "draw.h"
#include "font.h"

void startDraw(EPD_CLASS &epd, int32_t bgcolor) {
    epd.init(0);
    if (bgcolor >= 0) {
        epd.fillScreen(bgcolor);
    }
}

void endDraw(EPD_CLASS &epd, bool partial_update) {
    epd.display(partial_update);
    epd.hibernate();
}

// 照Minecraft抄的(逃
void drawCenteredString(U8G2_FOR_ADAFRUIT_GFX &u8g2, uint16_t x, uint16_t y, const char *str) {
    u8g2.drawUTF8(x - u8g2.getUTF8Width(str) / 2, y, str);
}

void drawTitleBar(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const char *title, bool sleeping, int8_t rssi, int8_t battery) {
    // Draw title
    u8g2.setFont(u8g2_font_wqy12_t);
    u8g2.drawUTF8(2, 12, title);
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
        u8g2.drawUTF8(x - 6, 12, "Z");
    }
    // Draw separator
    epd.drawFastHLine(0, 15, epd.width(), GxEPD_BLACK);
}

// x,y is the centre poistion of the arrow and asize is the radius out from the x,y position
// aangle is angle to draw the pointer at e.g. at 45° for NW
// pwidth is the pointer width in pixels
// plength is the pointer length in pixels
// copy from G6EJD/ESP32-e-Paper-Weather-Display, original author: David Bird
// XXX drawArrow 需要重写
void drawArrow(EPD_CLASS &epd, uint16_t x, uint16_t y, int16_t asize, float aangle, uint16_t pwidth, uint16_t plength) {
    float dx = (asize + 28) * cos((aangle - 90) * PI / 180) + x;
    float dy = (asize + 28) * sin((aangle - 90) * PI / 180) + y;
    float x1 = 0, y1 = plength;
    float x2 = pwidth / 2, y2 = pwidth / 2;
    float x3 = -pwidth / 2, y3 = pwidth / 2;
    float angle = aangle * PI / 180;
    float xx1 = x1 * cos(angle) - y1 * sin(angle) + dx;
    float yy1 = y1 * cos(angle) + x1 * sin(angle) + dy;
    float xx2 = x2 * cos(angle) - y2 * sin(angle) + dx;
    float yy2 = y2 * cos(angle) + x2 * sin(angle) + dy;
    float xx3 = x3 * cos(angle) - y3 * sin(angle) + dx;
    float yy3 = y3 * cos(angle) + x3 * sin(angle) + dy;
    epd.fillTriangle(xx1, yy1, xx3, yy3, xx2, yy2, GxEPD_BLACK);
}

// https://github.com/ricmoo/QRCode
void drawQRCode(EPD_CLASS &epd, uint16_t x, uint16_t y, uint8_t scale, const char *text, uint8_t version, uint8_t ecc) {
    QRCode qrcode;
    uint8_t buffer[qrcode_getBufferSize(version)];
    qrcode_initText(&qrcode, buffer, version, ecc, text);
    for (uint16_t dy = 0; dy < qrcode.size; dy++) {
        for (uint16_t dx = 0; dx < qrcode.size; dx++) {
            uint16_t color = qrcode_getModule(&qrcode, dx, dy) ? GxEPD_BLACK : GxEPD_WHITE;
            for (uint8_t i = 0; i < scale; i++) {
                for (uint8_t j = 0; j < scale; j++) {
                    epd.drawPixel(x + dx * scale + i, y + dy * scale + j, color);
                }
            }
        }
    }
}

const char *getWeatherIcon(uint16_t id, bool fill) {
    switch (id) {
        case 100: return !fill ? "\uf101" : "\uf1ac";
        case 101: return !fill ? "\uf102" : "\uf1ad";
        case 102: return !fill ? "\uf103" : "\uf1ae";
        case 103: return !fill ? "\uf104" : "\uf1af";
        case 104: return !fill ? "\uf105" : "\uf1b0";
        case 150: return !fill ? "\uf106" : "\uf1b1";
        case 151: return !fill ? "\uf107" : "\uf1b2";
        case 152: return !fill ? "\uf108" : "\uf1b3";
        case 153: return !fill ? "\uf109" : "\uf1b4";
        case 300: return !fill ? "\uf10a" : "\uf1b5";
        case 301: return !fill ? "\uf10b" : "\uf1b6";
        case 302: return !fill ? "\uf10c" : "\uf1b7";
        case 303: return !fill ? "\uf10d" : "\uf1b8";
        case 304: return !fill ? "\uf10e" : "\uf1b9";
        case 305: return !fill ? "\uf10f" : "\uf1ba";
        case 306: return !fill ? "\uf110" : "\uf1bb";
        case 307: return !fill ? "\uf111" : "\uf1bc";
        case 308: return !fill ? "\uf112" : "\uf1bd";
        case 309: return !fill ? "\uf113" : "\uf1be";
        case 310: return !fill ? "\uf114" : "\uf1bf";
        case 311: return !fill ? "\uf115" : "\uf1c0";
        case 312: return !fill ? "\uf116" : "\uf1c1";
        case 313: return !fill ? "\uf117" : "\uf1c2";
        case 314: return !fill ? "\uf118" : "\uf1c3";
        case 315: return !fill ? "\uf119" : "\uf1c4";
        case 316: return !fill ? "\uf11a" : "\uf1c5";
        case 317: return !fill ? "\uf11b" : "\uf1c6";
        case 318: return !fill ? "\uf11c" : "\uf1c7";
        case 350: return !fill ? "\uf11d" : "\uf1c8";
        case 351: return !fill ? "\uf11e" : "\uf1c9";
        case 399: return !fill ? "\uf11f" : "\uf1ca";
        case 400: return !fill ? "\uf120" : "\uf1cb";
        case 401: return !fill ? "\uf121" : "\uf1cc";
        case 402: return !fill ? "\uf122" : "\uf1cd";
        case 403: return !fill ? "\uf123" : "\uf1ce";
        case 404: return !fill ? "\uf124" : "\uf1cf";
        case 405: return !fill ? "\uf125" : "\uf1d0";
        case 406: return !fill ? "\uf126" : "\uf1d1";
        case 407: return !fill ? "\uf127" : "\uf1d2";
        case 408: return !fill ? "\uf128" : "\uf1d3";
        case 409: return !fill ? "\uf129" : "\uf1d4";
        case 410: return !fill ? "\uf12a" : "\uf1d5";
        case 456: return !fill ? "\uf12b" : "\uf1d6";
        case 457: return !fill ? "\uf12c" : "\uf1d7";
        case 499: return !fill ? "\uf12d" : "\uf1d8";
        case 500: return !fill ? "\uf12e" : "\uf1d9";
        case 501: return !fill ? "\uf12f" : "\uf1da";
        case 502: return !fill ? "\uf130" : "\uf1db";
        case 503: return !fill ? "\uf131" : "\uf1dc";
        case 504: return !fill ? "\uf132" : "\uf1dd";
        case 507: return !fill ? "\uf133" : "\uf1de";
        case 508: return !fill ? "\uf134" : "\uf1df";
        case 509: return !fill ? "\uf135" : "\uf1e0";
        case 510: return !fill ? "\uf136" : "\uf1e1";
        case 511: return !fill ? "\uf137" : "\uf1e2";
        case 512: return !fill ? "\uf138" : "\uf1e3";
        case 513: return !fill ? "\uf139" : "\uf1e4";
        case 514: return !fill ? "\uf13a" : "\uf1e5";
        case 515: return !fill ? "\uf13b" : "\uf1e6";
        case 800: return "\uf13c";
        case 801: return "\uf13d";
        case 802: return "\uf13e";
        case 803: return "\uf13f";
        case 804: return "\uf140";
        case 805: return "\uf141";
        case 806: return "\uf142";
        case 807: return "\uf143";
        case 900: return !fill ? "\uf144" : "\uf1e7";
        case 901: return !fill ? "\uf145" : "\uf1e8";
        case 999:
        default: return !fill ? "\uf146" : "\uf1e9";
    }
}
