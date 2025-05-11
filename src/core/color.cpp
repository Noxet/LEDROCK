#include "color.h"

Color::Color(uint8_t r, uint8_t g, uint8_t b) :
    r(r), g(g), b(b)
{

}

Color::Color(const std::string &hexCode)
{
    r = 4 * std::stoul(hexCode.substr(0, 2), nullptr, 16);
    g = 4 * std::stoul(hexCode.substr(2, 2), nullptr, 16);
    b = 4 * std::stoul(hexCode.substr(4, 2), nullptr, 16);
}
