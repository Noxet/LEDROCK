#include "led_controller.h"
#include "FreeRTOSConfig.h"
#include "message.h"
#include "core/color.h"

#include "core/sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "portmacro.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>


enum class LCSTATE
{
    IDLE,
    PULSE,
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
    std::optional<Event> current;
    while (1)
    {
        // fetch event if we are not currently handling one.
        if (!current && state == LCSTATE::IDLE)
        {
            Event tmp;
            if (xQueueReceive(m_queue, &tmp, 0) == pdPASS)
            {
                current = std::move(tmp);
            }
        }

        switch (state)
        {
            case LCSTATE::IDLE:
            {
                if (!current) break;

                const auto &ev = current.value();
                switch(ev.type)
                {
                    case MsgType::STATIC:
                        if (setStaticColor(ev.data.staticColor.color))
                        {
                            current.reset();
                        }
                        break;

                    case MsgType::FADE:
                        if (setFadeColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time))
                        {
                            current.reset();
                        }
                        break;

                    case MsgType::PULSE:
                        if (setPulseColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time, true))
                        {
                            // do not reset current event here since we need its data in PULSE state.
                            state = LCSTATE::PULSE;
                            printf("state -> pulse\n");
                        }
                        break;

                    case MsgType::NONE:
                        break;
                }
            } break;

            case LCSTATE::PULSE:
            {
                // we need a valid event while pulsing
                configASSERT(current.has_value());
                const auto &ev = current.value();

                Event tmp;
                if (xQueueReceive(m_queue, &tmp, 0) == pdPASS)
                {
                    // Preempt if a new state has arrived, so we don't have to wait for fading to finish.
                    current = std::move(tmp);
                    state = LCSTATE::IDLE;
                    break;
                }

                if (setPulseColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time, false))
                {
                    printf("re-fade\n");
                }
            } break;

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
    printf("fade: [%d, %d, %d] -> [%d, %d, %d]\n", from.rgb.r, from.rgb.g, from.rgb.b, to.rgb.r, to.rgb.g, to.rgb.b);
    bool ret = m_driver.setFadeColor(from, to, time);
    return ret;
    // printf("[%lld] fade done\n", getUptime());
}

bool LedController::setPulseColor(const Color &from, const Color &to, uint32_t time, bool reset)
{
    static Color _from = from;
    static Color _to = to;
    if (reset)
    {
        _from = from;
        _to = to;
    }

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


static std::string stateToString(const LCSTATE &state)
{
    switch (state)
    {
        case LCSTATE::IDLE: return "IDLE";
        case LCSTATE::PULSE: return "PULSE";
        case LCSTATE::CLEAR: return "CLEAR";
    }

    return "UNKNOWN STATE";
}


static void updateState(LCSTATE &state, const LCSTATE &newState)
{
    printf("[%s] -> [%s]\n", stateToString(state).c_str(), stateToString(newState).c_str());
}
