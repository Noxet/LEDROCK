#include "ColorMode.h"

void ColorMode::run()
{
	setupImpl();
	runImpl();
	cleanupImpl();
}

StaticColor::StaticColor(const RGB &rgb) : m_rgb(rgb)
{
}

void StaticColor::setupImpl()
{
	m_led.configure();
}

void StaticColor::runImpl()
{
	m_led.setRGB(m_rgb);
}
