#include "led_controller.h"
#include "message.h"
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
#include <cstring>
#include <iostream>
#include <optional>


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
    m_queue = xQueueCreate(10, sizeof(Event));
}


void LedController::init()
{
    m_driver.init();
}


void LedController::run()
{
    LCSTATE state = LCSTATE::IDLE;
    Event newEv;
    std::optional<Event> ev;
    while (1)
    {
        // non-blocking check if there is new messages
        if (xQueueReceive(m_queue, &newEv, 0) == pdPASS)
        {
            ev = newEv;
        }
        else
        {
            memset(&newEv, 0, sizeof(Event));
            newEv.type = MsgType::NONE;
        }

        switch (state)
        {
            case LCSTATE::IDLE:
                if (!ev) break;
                switch(ev->type)
                {
                    case MsgType::STATIC:
                        // if (setStaticColor(ev->data.staticColor.color))
                        // {
                        //     state = LCSTATE::IDLE;
                        //     ev.reset();
                        // }
                        state = LCSTATE::ONE_SHOT;
                        break;
                    case MsgType::FADE:
                        printf("Set fade color\n");
                        if (setFadeColor(ev->data.fadeColor.from, ev->data.fadeColor.to, ev->data.fadeColor.time))
                        {
                            state = LCSTATE::IDLE;
                            printf("fade success, clear ev\n");
                            ev.reset();
                        }
                        break;
                    case MsgType::NONE:
                        break;
                }
                break;
            case LCSTATE::ONE_SHOT:
                if (setStaticColor(ev->data.staticColor.color))
                {
                    state = LCSTATE::IDLE;
                    ev.reset();
                }
                break;
            case LCSTATE::REPEAT:
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

