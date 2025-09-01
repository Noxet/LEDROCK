#pragma once

#include "led_controller.h"

class UartCLI
{
public:
    UartCLI(LedController &controller);

    void poll();

private:
    LedController &m_controller;

    uint8_t m_buffer[256];
};
