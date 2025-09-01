#pragma once

#include "core/led_driver.h"

class LedController
{
public:
    LedController(ILedDriver &driver);

    void init();
    void setStaticColor(const Color &color);
    void setFadeColor(const Color &from, const Color &to, uint32_t time);
    void setBlinkColor(const Color &color1, const Color &color2, uint32_t time);

private:
    ILedDriver &m_driver;
};
