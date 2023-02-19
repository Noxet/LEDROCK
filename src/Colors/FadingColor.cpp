#include "ColorMode.h"
#include "Events/LEDEvent.h"

#include <algorithm>


FadingColor::FadingColor(const RGB &rgbFrom, const RGB &rgbTo, Timer &timer) : m_rgbFrom(rgbFrom), m_rgbTo(rgbTo), m_timer(timer)
{
}


void FadingColor::onEvent(Event *event)
{
	printf("[FadingColor] Event - %s\n", event->getName());

	if (auto ev = event_cast<LEDFadeCompleteEvent>(event))
	{
		printf("[FadingColor] - New fade\n");
		runImpl();
	}
}


void FadingColor::setupImpl()
{
	getLed().configure();
	getLed().enableFade();
}


void FadingColor::runImpl()
{
	getLed().setRGB(m_rgbFrom);
	getLed().startFade(m_rgbTo);

	std::swap(m_rgbFrom, m_rgbTo);
}


void FadingColor::cleanupImpl()
{
	getLed().disableFade();
}
