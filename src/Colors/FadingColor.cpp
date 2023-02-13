#include "ColorMode.h"

FadingColor::FadingColor(const RGB &rgb, Timer &timer) : m_rgb(rgb), m_timer(timer)
{
}


void FadingColor::setupImpl()
{
	m_led.configure();
	m_led.enableFade();
	//m_timer.init();
}


void FadingColor::runImpl()
{
	m_led.startFade(m_rgb);
	//m_timer.start();
}