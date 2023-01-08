#pragma once

#include "driver/timer.h"

#include <functional>

class Timer
{
	using Callback = bool (*)(void *);

public:
	Timer(int interval, Callback callback, bool repeat);

	void init();

	void start();
	void restart();
	void pause();
	void stop();

private:
	int m_interval;
	Callback m_callback;
	bool m_repeat;

	unsigned long long TIMER_DIVIDER = 8000;
	unsigned long long TIMER_SCALE = TIMER_BASE_CLK / TIMER_DIVIDER / 1000; // Scale in ms
};