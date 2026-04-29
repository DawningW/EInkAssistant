
// Inspired from FabioBatSilva/ArduinoFake

#include <chrono>
#include <thread>

#include "../Arduino.h"

static auto start_time = std::chrono::steady_clock::now();

void pinMode(uint8_t pin, uint8_t mode)
{
}

void digitalWrite(uint8_t pin, uint8_t val)
{
}

int digitalRead(uint8_t pin)
{
    return 0;
}

int analogRead(uint8_t pin)
{
    return 0;
}

void analogWrite(uint8_t pin, int val)
{
}

void analogReference(uint8_t mode)
{
}

unsigned long millis(void)
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

unsigned long micros(void)
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
}

void delay(unsigned long value)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(value));
}

void delayMicroseconds(unsigned int us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return 0;
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return 0;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
}

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
    return 0;
}

void detachInterrupt(uint8_t interruptNum)
{
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
}

void cli(void)
{
}

void sei(void)
{
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration)
{
}

void noTone(uint8_t pin)
{
}

long random(long max)
{
    return 0;
}

long random(long min, long max)
{
    return 0;
}

void randomSeed(unsigned long seed)
{
}

long map(long value, long fromLow, long fromHigh, long toLow, long toHigh)
{
    if (fromHigh == fromLow) {
        return toLow;
    }
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

void yield()
{
}
