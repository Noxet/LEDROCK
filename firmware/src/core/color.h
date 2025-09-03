#pragma once

#include <cstdint>
#include <format>
#include <string>

struct Color
{
    Color(uint8_t r, uint8_t g, uint8_t b);
    Color(const std::string &hexCode);

    uint8_t r;
    uint8_t g;
    uint8_t b;
};
