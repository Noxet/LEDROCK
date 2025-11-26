#pragma once

#include <cstdint>
#include <format>
#include <string>

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Color
{
    Color(uint8_t r, uint8_t g, uint8_t b);
    Color(RGB rgb);
    Color(const std::string &hexCode);

    RGB rgb;

    static std::string toString(RGB rgb);
};
