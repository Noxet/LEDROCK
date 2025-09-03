#include "led_controller.h"
#include "core/color.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <cstdint>
#include <iostream>

static const char *TAG = "LC";

enum class LCSTATE
{
    IDLE,
    ONE_SHOT,
    REPEAT,
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
        if (xQueueReceive(m_queue, &event, 10) != pdPASS)
        {
            event = 0;
        }
        switch (state)
        {
            case LCSTATE::IDLE:
                printf("idle state\n");
                if (event >= '0' && event < '9')
                {
                    state = LCSTATE::ONE_SHOT;
                }
                if (event == '9') state = LCSTATE::REPEAT;
                break;
            case LCSTATE::ONE_SHOT:
                printf("one shot state\n");
                setStaticColor(Color{255, 182, 78});
                state = LCSTATE::IDLE;
                break;
            case LCSTATE::REPEAT:
                printf("repeat state\n");
                if (event == '0')
                {
                    setStaticColor(Color{"000000"});
                    state = LCSTATE::IDLE;
                }
                else
                {
                    printf("setting pulse\n");
                    setPulseColor(Color{"FFD400"}, Color{"A0BD45"}, 3000);
                    printf("pulse done\n");
                }
                break;
        }
    }
}


void LedController::setStaticColor(const Color &color)
{
    m_driver.setStaticColor(color);
}


void LedController::setFadeColor(const Color &from, const Color &to, uint32_t time)
{
    setStaticColor(from);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    m_driver.setFadeColor(from, to, time);
}

void LedController::setPulseColor(const Color &from, const Color &to, uint32_t time)
{
    static Color _from = from;
    static Color _to = to;

    setFadeColor(_from, _to, time);

    Color tmp = _from;
    _from = _to;
    _to = tmp;
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

