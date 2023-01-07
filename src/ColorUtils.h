#pragma once

struct RGB
{
	RGB() {}
	RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}

	int r;
	int g;
	int b;
};