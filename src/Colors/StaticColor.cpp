#include "ColorMode.h"

StaticColor::StaticColor(const RGB &rgb) : m_rgb(rgb)
{
}


void StaticColor::setupImpl()
{
	getLed().configure();
}


void StaticColor::runImpl()
{
	getLed().setRGB(m_rgb);
}