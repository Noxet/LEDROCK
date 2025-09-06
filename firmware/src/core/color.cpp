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
    rgb.r = 4 * std::stoul(hexCode.substr(0, 2), nullptr, 16);
    rgb.g = 4 * std::stoul(hexCode.substr(2, 2), nullptr, 16);
    rgb.b = 4 * std::stoul(hexCode.substr(4, 2), nullptr, 16);
}
