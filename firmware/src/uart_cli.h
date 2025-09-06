#pragma once

#include "freertos/idf_additions.h"
#include "led_controller.h"
#include <cstdint>

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
    uint8_t read_u8_le();
    uint16_t read_u16_le();
    uint32_t read_u32_le();


private:
    QueueHandle_t &m_lcQueue;

    uint8_t m_buffer[256];
    uint8_t m_bufferDataSize;
    uint8_t *m_bufferReadPos;
    bool m_bufferToParse;
};


