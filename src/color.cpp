#include "color.h"

Color::Color(uint8_t r, uint8_t g, uint8_t b) :
    m_r(r), m_g(g), m_b(b)
{

}

Color::Color(const std::string &hexCode)
{
    m_r = 4 * std::stoul(hexCode.substr(0, 2), nullptr, 16);
    m_g = 4 * std::stoul(hexCode.substr(2, 2), nullptr, 16);
    m_b = 4 * std::stoul(hexCode.substr(4, 2), nullptr, 16);
}
