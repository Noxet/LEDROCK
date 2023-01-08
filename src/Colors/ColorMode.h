#pragma once

#include "Events/Event.h"

#include "Hardware/LED.h"
#include "Hardware/Timer.h"

#include <vector>

class ColorMode
{
public:
	virtual ~ColorMode() = default;

	void run()
	{
		setupImpl();
		runImpl();
		cleanupImpl();
	}

	virtual void onEvent(Event *event) {}

protected:
	LED m_led;

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
};


class SwitchingColor : public ColorMode
{
public:
	explicit SwitchingColor(std::vector<RGB> &colors, Timer &timer);

	virtual void onEvent(Event *event) override;

private:
	virtual void setupImpl() override;
	virtual void runImpl() override;

	void next();

	int m_currentColor;
	std::vector<RGB> m_colors;
	Timer m_timer;
};