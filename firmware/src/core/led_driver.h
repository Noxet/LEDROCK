#pragma once

#include "color.h"
#include <cstdint>

class ILedDriver
{
public:
    virtual void init(void) = 0;
    virtual bool setStaticColor(const Color &color) = 0;
    virtual bool setFadeColor(const Color &from, const Color &to, uint32_t time) = 0;
};
