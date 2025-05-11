#pragma once

#include "core/led_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"

class Ledstrip : public ILedDriver
{
public:
    Ledstrip();

    void init() override;
    void setStaticColor(const Color &color) override;
    void setFadeColor(const Color &from, const Color &to, uint32_t time) override;

private:

    struct ledcChannels
    {
        ledc_channel_config_t r;
        ledc_channel_config_t g;
        ledc_channel_config_t b;
    };

    ledcChannels m_ledConfig;
    SemaphoreHandle_t m_ledSem;

    static bool ledcFadeEndCB(const ledc_cb_param_t *param, void *userArg);
};
