#include "Timer.h"


Timer::Timer(int interval, Callback callback, bool repeat) : m_interval(interval), m_callback(callback), m_repeat(repeat)
{
}

void Timer::init()
{
    timer_config_t config;
    config.alarm_en = TIMER_ALARM_EN;
    config.auto_reload = m_repeat ? TIMER_AUTORELOAD_EN : TIMER_AUTORELOAD_DIS;
    config.counter_dir = TIMER_COUNT_UP;
    config.divider = TIMER_DIVIDER;
    config.intr_type = TIMER_INTR_LEVEL;
    config.counter_en = TIMER_PAUSE;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, m_interval * TIMER_SCALE);

    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, m_callback, nullptr, 0);
}


void Timer::start()
{
    timer_start(TIMER_GROUP_0, TIMER_0);
}


void Timer::restart()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm(TIMER_GROUP_0, TIMER_0, TIMER_ALARM_EN);
}


void Timer::pause()
{
}


void Timer::stop()
{
}
