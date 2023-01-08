#include "ColorMode.h"
#include "Events/TimerEvent.h"


SwitchingColor::SwitchingColor(std::vector<RGB> &colors) : m_currentColor(0), m_colors(colors)
{
}


void SwitchingColor::onEvent(Event *event)
{
	printf("[SwitchingColor] - %s\n", event->getName());

	if (auto ev = event_cast<TimerAlarmEvent>(event))
	{
		printf("[SwitchingColor] - Change color\n");

		m_currentColor = (m_currentColor + 1) % m_colors.size();
		m_led.setRGB(m_colors.at(m_currentColor));
	}
}


void SwitchingColor::setupImpl()
{
	m_led.configure();
}


void SwitchingColor::runImpl()
{
	m_led.setRGB(m_colors.at(m_currentColor));
}
