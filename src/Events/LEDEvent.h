#pragma once

#include "Event.h"

class LEDFadeCompleteEvent : public Event
{
public:
	LEDFadeCompleteEvent(int timerId) : m_timerId(timerId) {}
	int getTimerId() { return m_timerId; }

	EVENT_TYPE(ledFadeComplete)

private:
	int m_timerId;
};