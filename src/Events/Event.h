#pragma once

enum class EventType
{
	buttonPressed,
	timerAlarm,
	ledFadeComplete,
};


#define EVENT_TYPE(type) static EventType getStaticEventType() { return EventType::type; } \
						virtual EventType getEventType() const override { return getStaticEventType(); } \
						virtual const char *getName() const override { return #type; }


class Event
{
public:
	virtual ~Event() = default;

	virtual EventType getEventType() const = 0;
	virtual const char *getName() const = 0;
};


template <typename T>
T *event_cast(Event *ev)
{
	if (ev->getEventType() == T::getStaticEventType())
	{
		return static_cast<T *>(ev);
	}

	return nullptr;
}