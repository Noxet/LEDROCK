#pragma once

#include "led_controller.h"

class UartCLI
{
public:
    UartCLI(LedController &controller);

    void poll();
    bool hasPacket();
    void parse();

    static inline bool isLineEnding(uint8_t ch)
    {
        if (ch == '\n' || ch == '\r') return true;
        return false;
    }

private:
    LedController &m_controller;

    uint8_t m_buffer[256];
    uint8_t m_bufferPos;
    bool m_bufferToParse;
};
