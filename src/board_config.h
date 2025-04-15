// SMPL_2021.7.22_V2 开发板
#ifdef BOARD_SMPL_V2
#define HAS_CONFIG
#define EPD_CS 15
#define EPD_DC 0
#define EPD_RST 2
#define EPD_BUSY 4
#define EPD_ROTATION 3
#define SUPPORT_PARTIAL_UPDATE false
#define KEY_PIN_MODE INPUT_PULLUP
#define KEY_SWITCH 5
#define KEY_TRIGGER_LEVEL LOW
#define SUPPORT_DEEP_SLEEP false
#endif

// 由 OurEDA 设计的 CoreBoard_ESP32 开发板
#ifdef BOARD_COREBOARD_V1
#define HAS_CONFIG
#define EPD_CS 5
#define EPD_DC 6
#define EPD_RST 7
#define EPD_BUSY 8
#define EPD_CLK 4
#define EPD_MOSI 10
#define EPD_ROTATION 3
#define SUPPORT_PARTIAL_UPDATE false
#define KEY_PIN_MODE INPUT_PULLUP
#define KEY_SWITCH 0
#define KEY_TRIGGER_LEVEL LOW
#define SUPPORT_DEEP_SLEEP true
#endif
