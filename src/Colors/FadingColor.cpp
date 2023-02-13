#include "ColorMode.h"
#include "Events/LEDEvent.h"


FadingColor::FadingColor(const RGB &rgbFrom, const RGB &rgbTo, Timer &timer) : m_rgbFrom(rgbFrom), m_rgbTo(rgbTo), m_timer(timer)
{
}


void FadingColor::onEvent(Event *event)
{
	printf("[FadingColor] Event - %s\n", event->getName());

	if (auto ev = event_cast<LEDFadeCompleteEvent>(event))
	{
		printf("[FadingColor] - Change color\n");
	}
}


void FadingColor::setupImpl()
{
	m_led.configure();
	m_led.enableFade();
}


void FadingColor::runImpl()
{
	printf("[FadingColor] - Run\n");

	m_led.setRGB(m_rgbFrom);
	m_led.startFade(m_rgbTo);
}


void FadingColor::cleanupImpl()
{
	m_led.disableFade();
}
