#pragma once

#include <memory>
#include <vector>

#include "Colors/ColorMode.h"
#include "Events/Event.h"

class ColorManager
{
public:
	ColorManager& addColorMode(std::unique_ptr<ColorMode> cm);

	void start();
	void onEvent(Event *event);

private:
	void nextColorMode();

	std::vector<std::unique_ptr<ColorMode>> m_colorModes;
	unsigned int m_currentColorMode{ 0 };
};