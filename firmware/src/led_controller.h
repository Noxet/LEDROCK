#pragma once

#include "core/led_driver.h"
#include "freertos/idf_additions.h"

class LedController
{
public:
    LedController(ILedDriver &driver);

    void init();

    // Need to be static (or non-class func) to be passed to xTaskCreate
    static void ledControllerTask(void *pvParam);

private:
    void run();
    void setStaticColor(const Color &color);
    void setFadeColor(const Color &from, const Color &to, uint32_t time);
    void setPulseColor(const Color &from, const Color &to, uint32_t time);
    void setBlinkColor(const Color &color1, const Color &color2, uint32_t time);


public:
    QueueHandle_t m_queue;

private:
    ILedDriver &m_driver;
};
