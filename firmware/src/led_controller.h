#pragma once

#include "core/led_driver.h"
#include "freertos/idf_additions.h"

class LedController
{
public:
    LedController(ILedDriver &driver);

    // Need to be static (or non-class func) to be passed to xTaskCreate
    static void ledControllerTask(void *pvParam);
    QueueHandle_t getQueue();

private:
    void init();
    void run();
    bool setStaticColor(const Color &color);
    bool setFadeColor(const Color &from, const Color &to, uint32_t time);
    bool setPulseColor(const Color &from, const Color &to, uint32_t time, bool reset);
    bool setBlinkColor(const Color &color1, const Color &color2, uint32_t time);

private:
    ILedDriver &m_driver;
    QueueHandle_t m_queue;
};
