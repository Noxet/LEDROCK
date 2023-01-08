#include "ColorManager.h"
#include "Events/ButtonEvent.h"

ColorManager& ColorManager::addColorMode(std::unique_ptr<ColorMode> cm)
{
	m_colorModes.emplace_back(move(cm));
	//m_colorModes.back()->run();
	return *this;
}

void ColorManager::start()
{
	m_colorModes.at(m_currentColorMode)->run();
}

void ColorManager::onEvent(Event *event)
{
	printf("[ColorManager] - %s\n", event->getName());

	if (auto ev = event_cast<ButtonPressedEvent>(event))
	{
		printf("[ColorManager] - Next color mode\n");

		nextColorMode();
		return;
	}

	// Let the color mode handle the other events
	m_colorModes.at(m_currentColorMode)->onEvent(event);
}

void ColorManager::nextColorMode()
{
	m_currentColorMode = (m_currentColorMode + 1) % m_colorModes.size();
	m_colorModes.at(m_currentColorMode)->run();
}
