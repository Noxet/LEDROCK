#pragma once

#include "Event.h"

class LEDFadeCompleteEvent : public Event
{
public:
	LEDFadeCompleteEvent() {}

	EVENT_TYPE(ledFadeComplete)
};