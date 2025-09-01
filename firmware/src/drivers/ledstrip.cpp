#include "ledstrip.h"
#include "driver/ledc.h"
#include "freertos/idf_additions.h"
#include "hal/ledc_types.h"
#include "portmacro.h"


constexpr int GPIO_R = 3;
constexpr int GPIO_G = 4;
constexpr int GPIO_B = 5;

constexpr uint32_t LED_COLOR_SCALE = 32;


Ledstrip::Ledstrip()
{
    m_ledConfig.r = {
        .gpio_num = GPIO_R,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = { .output_invert = 0 },
    };

    m_ledConfig.g = {
        .gpio_num = GPIO_G,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = { .output_invert = 0 },
    };

    m_ledConfig.b = {
        .gpio_num = GPIO_B,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_2,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = { .output_invert = 0 },
    };

    m_ledSem = xSemaphoreCreateCounting(3, 3);
}


void Ledstrip::init()
{
    ledc_timer_t timers[] = {LEDC_TIMER_0, LEDC_TIMER_1, LEDC_TIMER_2};
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = 0,
    };

    for (auto tim : timers)
    {
        ledc_timer.timer_num = tim;
        ledc_timer_config(&ledc_timer);
    }

    ledc_channel_config(&m_ledConfig.r);
    ledc_channel_config(&m_ledConfig.g);
    ledc_channel_config(&m_ledConfig.b);


    ledc_fade_func_install(0);
    ledc_cbs_t callbacks = {
        .fade_cb = &Ledstrip::ledcFadeEndCB,
    };

    ledc_cb_register(m_ledConfig.r.speed_mode, m_ledConfig.r.channel, &callbacks, m_ledSem);
    ledc_cb_register(m_ledConfig.g.speed_mode, m_ledConfig.g.channel, &callbacks, m_ledSem);
    ledc_cb_register(m_ledConfig.b.speed_mode, m_ledConfig.b.channel, &callbacks, m_ledSem);
}


void Ledstrip::setStaticColor(const Color &color)
{
    ledc_set_duty(m_ledConfig.r.speed_mode, m_ledConfig.r.channel, color.r * LED_COLOR_SCALE);
    ledc_set_duty(m_ledConfig.g.speed_mode, m_ledConfig.g.channel, color.g * LED_COLOR_SCALE);
    ledc_set_duty(m_ledConfig.b.speed_mode, m_ledConfig.b.channel, color.b * LED_COLOR_SCALE);
    ledc_update_duty(m_ledConfig.r.speed_mode, m_ledConfig.r.channel);
    ledc_update_duty(m_ledConfig.g.speed_mode, m_ledConfig.g.channel);
    ledc_update_duty(m_ledConfig.b.speed_mode, m_ledConfig.b.channel);
}


void Ledstrip::setFadeColor(const Color &from, const Color &to, uint32_t time)
{
    for (int i = 0; i < 3; ++i) xSemaphoreTake(m_ledSem, portMAX_DELAY);
    ledc_set_fade_time_and_start(m_ledConfig.r.speed_mode, m_ledConfig.r.channel, to.r * LED_COLOR_SCALE, time, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(m_ledConfig.g.speed_mode, m_ledConfig.g.channel, to.g * LED_COLOR_SCALE, time, LEDC_FADE_NO_WAIT);
    ledc_set_fade_time_and_start(m_ledConfig.b.speed_mode, m_ledConfig.b.channel, to.b * LED_COLOR_SCALE, time, LEDC_FADE_NO_WAIT);
}


bool Ledstrip::ledcFadeEndCB(const ledc_cb_param_t *param, void *userArg)
{
    BaseType_t taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT)
    {
        SemaphoreHandle_t sem = (SemaphoreHandle_t) userArg;
        xSemaphoreGiveFromISR(sem, &taskAwoken);
    }

    return taskAwoken;
}
