#include "Wire.h"

void TwoWire::begin(void) {}

void TwoWire::begin(uint8_t address) {}

void TwoWire::begin(int address) {}

void TwoWire::end(void) {}

void TwoWire::setClock(uint32_t clock) {}

void TwoWire::setWireTimeout(uint32_t timeout, bool reset_with_timeout) {}

bool TwoWire::getWireTimeoutFlag(void) {
    return false;
}

void TwoWire::clearWireTimeoutFlag(void) {}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint32_t iaddress, uint8_t isize,
                             uint8_t sendStop) {
    return 0;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity,
                             uint8_t sendStop) {
    return 0;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity) {
    return 0;
}

void TwoWire::beginTransmission(uint8_t address) {}

void TwoWire::beginTransmission(int address) {}

uint8_t TwoWire::endTransmission(bool sendStop) {
    return 0;
}

uint8_t TwoWire::endTransmission(void) {
    return endTransmission(true);
}

size_t TwoWire::write(uint8_t data) {
    return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity) {
    return quantity;
}

int TwoWire::available(void) {
    return 0;
}

int TwoWire::read(void) {
    return -1;
}

int TwoWire::peek(void) {
    return 0;
}

void TwoWire::flush(void) {}

void TwoWire::onReceive(void (*function)(int)) {}

void TwoWire::onRequest(void (*function)(void)) {}

TwoWire Wire;
