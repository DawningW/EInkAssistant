#include "../Arduino.h"
#include "../HardwareSerial.h"

HardwareSerial::HardwareSerial(
    volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
    volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
    volatile uint8_t *ucsrc, volatile uint8_t *udr) : _ubrrh(ubrrh), _ubrrl(ubrrl),
                                                      _ucsra(ucsra), _ucsrb(ucsrb), _ucsrc(ucsrc),
                                                      _udr(udr),
                                                      _rx_buffer_head(0), _rx_buffer_tail(0),
                                                      _tx_buffer_head(0), _tx_buffer_tail(0)
{
}

void HardwareSerial::begin(unsigned long baud, byte config)
{
}

void HardwareSerial::end()
{
}

int HardwareSerial::available(void)
{
    return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE;
}

int HardwareSerial::peek(void)
{
    if (_rx_buffer_head == _rx_buffer_tail)
    {
        return -1;
    }
    else
    {
        return _rx_buffer[_rx_buffer_tail];
    }
}

int HardwareSerial::read(void)
{
    // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buffer_head == _rx_buffer_tail)
    {
        return -1;
    }
    else
    {
        unsigned char c = _rx_buffer[_rx_buffer_tail];
        _rx_buffer_tail = (rx_buffer_index_t)(_rx_buffer_tail + 1) % SERIAL_RX_BUFFER_SIZE;
        return c;
    }
}

int HardwareSerial::availableForWrite(void)
{
    tx_buffer_index_t head;
    tx_buffer_index_t tail;

    head = _tx_buffer_head;
    tail = _tx_buffer_tail;
    if (head >= tail)
        return SERIAL_TX_BUFFER_SIZE - 1 - head + tail;
    return tail - head - 1;
}

void HardwareSerial::flush()
{
}

size_t HardwareSerial::write(uint8_t c)
{
    _written = true;
    putchar(c);

    return 1;
}

HardwareSerial Serial(0, 0, 0, 0, 0, 0);
#if defined(UBRRH)
#endif

#if defined(USBCON)
#endif // USBCON