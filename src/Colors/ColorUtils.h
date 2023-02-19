#pragma once

#include <string>

struct RGB
{
	RGB() {}
	RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
	RGB(const std::string &hexCode)
	{
		r = 4 * std::stoul(hexCode.substr(0, 2), nullptr, 16);
		g = 4 * std::stoul(hexCode.substr(2, 2), nullptr, 16);
		b = 4 * std::stoul(hexCode.substr(4, 2), nullptr, 16);
	}

	int r;
	int g;
	int b;
};