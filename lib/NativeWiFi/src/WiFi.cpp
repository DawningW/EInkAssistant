#include "WiFi.h"

int32_t WiFiClass::RSSI() {
    return -50;
}

IPAddress WiFiClass::localIP() {
    return IPAddress(127, 0, 0, 1);
}

IPAddress WiFiClass::subnetMask() {
    return IPAddress(255, 255, 255, 0);
}

IPAddress WiFiClass::gatewayIP() {
    return IPAddress(127, 0, 0, 1);
}

WiFiClass WiFi;
