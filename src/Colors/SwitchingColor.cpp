#include "ColorMode.h"
#include "Events/TimerEvent.h"


SwitchingColor::SwitchingColor(std::vector<RGB> &colors, Timer &timer) : m_currentColor(0), m_colors(colors), m_timer(timer)
{
}


void SwitchingColor::onEvent(Event *event)
{
	printf("[SwitchingColor] - %s\n", event->getName());

	if (auto ev = event_cast<TimerAlarmEvent>(event))
	{
		printf("[SwitchingColor] - Change color\n");
		next();
	}
}


void SwitchingColor::setupImpl()
{
	getLed().configure();
	m_timer.init();
}


void SwitchingColor::runImpl()
{
	getLed().setRGB(m_colors.at(m_currentColor));
	m_timer.start();
}


void SwitchingColor::next()
{
	m_currentColor = (m_currentColor + 1) % m_colors.size();
	getLed().setRGB(m_colors.at(m_currentColor));
}
