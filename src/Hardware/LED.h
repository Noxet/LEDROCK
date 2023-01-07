#pragma once

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "ColorUtils.h"

constexpr int g_gpioR = 16;
constexpr int g_gpioG = 17;
constexpr int g_gpioB = 5;

constexpr int g_clkFrequency = 5000;


class LED
{
public:
	LED();

	void configure();
	void setRGB(const RGB &rgb);

private:
	int m_gpioR;
	int m_gpioG;
	int m_gpioB;

	RGB m_rgb;
};