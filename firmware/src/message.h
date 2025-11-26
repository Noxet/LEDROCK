#pragma once

#include "core/color.h"

#include <cstdint>

enum class MsgType : uint8_t
{
    NONE = 0,
    STATIC,
    FADE,
    PULSE,
};


struct StaticColor
{
    RGB color;
};

struct FadeColor
{
    RGB from;
    RGB to;
    uint32_t time;
};


struct Event
{
    MsgType type;
    union
    {
        StaticColor staticColor;
        FadeColor fadeColor;
    } data;
};

static_assert(std::is_trivially_copyable_v<Event>, "Must be POD for FreeRTOS queue");
