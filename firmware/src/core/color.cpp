#include "color.h"

Color::Color(uint8_t r, uint8_t g, uint8_t b) :
    rgb(r, g, b)
{

}


Color::Color(RGB rgb) : rgb(rgb)
{

}


Color::Color(const std::string &hexCode)
{
    std::string hex = hexCode;
    if (hexCode.at(0) == '#') hex = hex.substr(1);
    rgb.r = std::stoul(hex.substr(0, 2), nullptr, 16);
    rgb.g = std::stoul(hex.substr(2, 2), nullptr, 16);
    rgb.b = std::stoul(hex.substr(4, 2), nullptr, 16);
}


std::string Color::toString(RGB rgb)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%02x%02x%02x", rgb.r, rgb.g, rgb.b);
    return std::string(buf);
}
