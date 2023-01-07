#include "LED.h"

LED::LED() : m_gpioR(g_gpioR), m_gpioG(g_gpioG), m_gpioB(g_gpioB)
{
}

void LED::configure()
{
    // Set up PWM timer
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = g_clkFrequency,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    // Set up LED channels
    ledc_channel_config_t led_conf[3] = {
        {
            .gpio_num = m_gpioR,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,

        },
        {
            .gpio_num = m_gpioG,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_1,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,

        },
        {
            .gpio_num = m_gpioB,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_2,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0
        }
    };

    for (int i = 0; i < 3; i++)
    {
        ledc_channel_config(&led_conf[i]);
    }
}

void LED::setRGB(const RGB &rgb)
{
    m_rgb = rgb;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, m_rgb.r);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, m_rgb.g);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, m_rgb.b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

