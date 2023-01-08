#include "ColorManager.h"
#include "Events/ButtonEvent.h"

ColorManager& ColorManager::addColorMode(std::unique_ptr<ColorMode> cm)
{
	m_colorModes.emplace_back(move(cm));
	m_colorModes.back()->run();
	return *this;
}

void ColorManager::onEvent(Event *event)
{
	if (auto ev = event_cast<ButtonPressedEvent>(event))
	{
		nextColorMode();
		return;
	}


}

void ColorManager::nextColorMode()
{
	m_currentColorMode = (m_currentColorMode + 1) % m_colorModes.size();
	m_colorModes.at(m_currentColorMode)->run();
}
