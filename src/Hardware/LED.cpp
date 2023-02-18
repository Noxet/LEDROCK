#include "LED.h"

LED::LED(ledc_cbs_t *callback) : m_gpioR(g_gpioR), m_gpioG(g_gpioG), m_gpioB(g_gpioB), mp_callbacks(callback)
{
    m_ledConf[0].gpio_num = m_gpioR;
    m_ledConf[0].speed_mode = LEDC_LOW_SPEED_MODE;
    m_ledConf[0].channel = LEDC_CHANNEL_0;
    m_ledConf[0].intr_type = LEDC_INTR_DISABLE;
    m_ledConf[0].timer_sel = LEDC_TIMER_0;
    m_ledConf[0].duty = 0;
    m_ledConf[0].hpoint = 0;

    m_ledConf[1].gpio_num = m_gpioG;
    m_ledConf[1].speed_mode = LEDC_LOW_SPEED_MODE;
    m_ledConf[1].channel = LEDC_CHANNEL_1;
    m_ledConf[1].intr_type = LEDC_INTR_DISABLE;
    m_ledConf[1].timer_sel = LEDC_TIMER_0;
    m_ledConf[1].duty = 0;
    m_ledConf[1].hpoint = 0;

    m_ledConf[2].gpio_num = m_gpioB;
    m_ledConf[2].speed_mode = LEDC_LOW_SPEED_MODE;
    m_ledConf[2].channel = LEDC_CHANNEL_2;
    m_ledConf[2].intr_type = LEDC_INTR_DISABLE;
    m_ledConf[2].timer_sel = LEDC_TIMER_0;
    m_ledConf[2].duty = 0;
    m_ledConf[2].hpoint = 0;
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
    //ledc_channel_config_t m_ledConf[3] = {
    //    {
    //        .gpio_num = m_gpioR,
    //        .speed_mode = LEDC_LOW_SPEED_MODE,
    //        .channel = LEDC_CHANNEL_0,
    //        .intr_type = LEDC_INTR_DISABLE,
    //        .timer_sel = LEDC_TIMER_0,
    //        .duty = 0,

    //    },
    //    {
    //        .gpio_num = m_gpioG,
    //        .speed_mode = LEDC_LOW_SPEED_MODE,
    //        .channel = LEDC_CHANNEL_1,
    //        .intr_type = LEDC_INTR_DISABLE,
    //        .timer_sel = LEDC_TIMER_0,
    //        .duty = 0,

    //    },
    //    {
    //        .gpio_num = m_gpioB,
    //        .speed_mode = LEDC_LOW_SPEED_MODE,
    //        .channel = LEDC_CHANNEL_2,
    //        .intr_type = LEDC_INTR_DISABLE,
    //        .timer_sel = LEDC_TIMER_0,
    //        .duty = 0
    //    }
    //};

    for (int i = 0; i < 3; i++)
    {
        ledc_channel_config(&m_ledConf[i]);
        ledc_cb_register(m_ledConf[i].speed_mode, m_ledConf[i].channel, mp_callbacks, nullptr);
    }
}


void LED::enableFade()
{
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
}


void LED::disableFade()
{
    ledc_fade_func_uninstall();
}


void LED::startFade(const RGB &rgb)
{
    m_rgb = rgb;

    //ESP_ERROR_CHECK(ledc_set_fade_with_time(m_ledConf[0].speed_mode, m_ledConf[0].channel, m_rgb.r, 5000));
    //ESP_ERROR_CHECK(ledc_fade_start(m_ledConf[0].speed_mode, m_ledConf[0].channel, LEDC_FADE_NO_WAIT));

    ESP_ERROR_CHECK(ledc_set_fade_time_and_start(m_ledConf[0].speed_mode, m_ledConf[0].channel, m_rgb.r, 5000, LEDC_FADE_NO_WAIT));
    ESP_ERROR_CHECK(ledc_set_fade_time_and_start(m_ledConf[1].speed_mode, m_ledConf[1].channel, m_rgb.g, 5000, LEDC_FADE_NO_WAIT));
    ESP_ERROR_CHECK(ledc_set_fade_time_and_start(m_ledConf[2].speed_mode, m_ledConf[2].channel, m_rgb.b, 5000, LEDC_FADE_NO_WAIT));
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

