
#include <cstdint>
#include <string>

class Color
{
public:
    Color(uint8_t r, uint8_t g, uint8_t b);
    Color(const std::string &hexCode);

private:
    uint8_t m_r;
    uint8_t m_g;
    uint8_t m_b;
};
