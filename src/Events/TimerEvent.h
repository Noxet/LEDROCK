#include "Event.h"

class TimerAlarmEvent : public Event
{
public:
	TimerAlarmEvent(int timerId) : m_timerId(timerId) {}
	int getTimerId() { return m_timerId; }

	EVENT_TYPE(timerAlarm)

private:
	int m_timerId;
};