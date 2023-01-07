#pragma once

#include "Event.h"

class ButtonPressedEvent : public Event
{
public:
	ButtonPressedEvent(int test) : m_test(test) { }
	int getTest() { return m_test; }

	EVENT_TYPE(buttonPressed)

private:
	int m_test;
};