#pragma once

#include "Hardware/LED.h"

class ColorMode
{
public:
	virtual ~ColorMode() = default;

	void run();

private:
	virtual void setupImpl() {}
	virtual void runImpl() = 0;
	virtual void cleanupImpl() {}
};


class StaticColor : public ColorMode
{
public:
	explicit StaticColor(const RGB &rgb);

private:
	virtual void setupImpl() override;
	virtual void runImpl() override;

	RGB m_rgb;
	LED m_led;
};