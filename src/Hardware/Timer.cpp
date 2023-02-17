#include "Timer.h"


Timer::Timer(int interval, Callback callback, bool repeat, timer_idx_t timerId) : m_interval(interval), m_callback(callback), m_repeat(repeat), m_timerId(timerId)
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
    timer_init(TIMER_GROUP_0, m_timerId, &config);

    timer_set_counter_value(TIMER_GROUP_0, m_timerId, 0);
    timer_set_alarm_value(TIMER_GROUP_0, m_timerId, m_interval * TIMER_SCALE);

    timer_enable_intr(TIMER_GROUP_0, m_timerId);
    timer_isr_callback_add(TIMER_GROUP_0, m_timerId, m_callback, nullptr, 0);
}


void Timer::start()
{
    timer_start(TIMER_GROUP_0, m_timerId);
}


void Timer::restart()
{
    timer_set_counter_value(TIMER_GROUP_0, m_timerId, 0);
    timer_set_alarm(TIMER_GROUP_0, m_timerId, TIMER_ALARM_EN);
    start();
}


void Timer::pause()
{
}


void Timer::stop()
{
}
