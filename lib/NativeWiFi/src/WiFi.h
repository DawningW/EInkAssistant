#ifndef WiFi_h
#define WiFi_h

#include "IPAddress.h"

class WiFiClass {
public:
    int32_t RSSI();
    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
};

extern WiFiClass WiFi;

#endif
