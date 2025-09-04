#include "led_controller.h"
#include "core/color.h"

#include "core/sys.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <cstdint>
#include <iostream>


enum class LCSTATE
{
    IDLE,
    ONE_SHOT,
    REPEAT,
    CLEAR,
};


LedController::LedController(ILedDriver &driver)
    : m_driver(driver)
{
    m_queue = xQueueCreate(10, sizeof(uint8_t));
}


void LedController::init()
{
    m_driver.init();
}


void LedController::run()
{
    LCSTATE state = LCSTATE::IDLE;
    uint8_t event;
    while (1)
    {
        // non-blocking check if there is new messages
        if (xQueueReceive(m_queue, &event, 0) != pdPASS)
        {
            event = 0;
        }

        switch (state)
        {
            case LCSTATE::IDLE:
                if (event > '0' && event < '9')
                {
                    state = LCSTATE::ONE_SHOT;
                }
                if (event == '9') state = LCSTATE::REPEAT;
                if (event == '0') state = LCSTATE::CLEAR;
                break;
            case LCSTATE::ONE_SHOT:
                if (setStaticColor(Color{255, 182, 78}))
                {
                    state = LCSTATE::IDLE;
                }
                break;
            case LCSTATE::REPEAT:
                if (event == '0')
                {
                    state = LCSTATE::CLEAR;
                }
                else
                {
                    setPulseColor(Color{"FFD400"}, Color{"A0BD45"}, 3000);
                }
                break;
            case LCSTATE::CLEAR:
                if (setStaticColor(Color{"000000"}))
                {
                    state = LCSTATE::IDLE;
                }
                break;
        }

        // pet the dog
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


bool LedController::setStaticColor(const Color &color)
{
    // printf("[%lld] set static\n", getUptime());
    bool ret = m_driver.setStaticColor(color);
    return ret;
    // printf("[%lld] static done\n", getUptime());
}


bool LedController::setFadeColor(const Color &from, const Color &to, uint32_t time)
{
    // printf("[%lld] set fade\n", getUptime());
    setStaticColor(from);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    bool ret = m_driver.setFadeColor(from, to, time);
    return ret;
    // printf("[%lld] fade done\n", getUptime());
}

bool LedController::setPulseColor(const Color &from, const Color &to, uint32_t time)
{
    static Color _from = from;
    static Color _to = to;

    bool ret = setFadeColor(_from, _to, time);

    // swap so we can fade the other way the next time the function is called.
    Color tmp = _from;
    _from = _to;
    _to = tmp;
    return ret;
}


/**
 * Just grab the led controller from the passed param, and run the inf loop
 */
void LedController::ledControllerTask(void *pvParam)
{
    // TODO: Add logging over wifi, and abort
    assert(pvParam);
    LedController *lc = static_cast<LedController *>(pvParam);
    lc->run();
}

