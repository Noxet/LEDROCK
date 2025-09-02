#pragma once

#include "freertos/idf_additions.h"
#include "led_controller.h"

class UartCLI
{
public:
    UartCLI(QueueHandle_t &lcQueue);

    void poll();
    bool hasPacket();
    void parse();

    static inline bool isLineEnding(uint8_t ch)
    {
        if (ch == '\n' || ch == '\r') return true;
        return false;
    }

private:
    QueueHandle_t &m_lcQueue;

    uint8_t m_buffer[256];
    uint8_t m_bufferPos;
    bool m_bufferToParse;
};
