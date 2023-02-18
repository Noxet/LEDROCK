#pragma once

#include "Events/Event.h"

#include "Utils.h"
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


class FadingColor : public ColorMode
{
public:
	explicit FadingColor(const RGB &rgbFrom, const RGB &rgbTo, Timer &timer);

	virtual void onEvent(Event *event) override;

private:
	virtual void setupImpl() override;
	virtual void runImpl() override;
	virtual void cleanupImpl() override;

	RGB m_rgbFrom;
	RGB m_rgbTo;
	Timer m_timer;
};