#pragma once

#include <memory>
#include <vector>

#include "ColorMode.h"

class ColorManager
{
public:
	ColorManager& addColorMode(std::unique_ptr<ColorMode> cm);
	void nextColorMode();

private:
	std::vector<std::unique_ptr<ColorMode>> m_colorModes;
	unsigned int m_currentColorMode{ 0 };
};