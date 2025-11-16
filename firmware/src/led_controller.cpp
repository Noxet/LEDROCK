#include "led_controller.h"
#include "core/log.h"
#include "core/sys.h"
#include "message.h"
#include "core/color.h"

#include "FreeRTOSConfig.h"
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


static const char *TAG = "LED_CTRL";


enum class LCSTATE
{
    IDLE,
    PULSE,
    CLEAR,
};

static void updateState(LCSTATE &state, const LCSTATE &newState);
static constexpr const char *eventToString(const Event &ev);


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
                if (current) LRLOGI("Got new event: %s\n", eventToString(current.value()));
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
                            ESP_LOGI(TAG, "Set staticcolor done");
                            LRLOGI("SET STATIC DONE\n");
                            current.reset();
                        }
                        break;

                    case MsgType::FADE:
                        if (setFadeColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time))
                        {
                            ESP_LOGI(TAG, "Set fadecolor done");
                            current.reset();
                        }
                        break;

                    case MsgType::PULSE:
                        if (setPulseColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time, true))
                        {
                            // do not reset current event here since we need its data in PULSE state.
                            ESP_LOGI(TAG, "Set pulsecolor done");
                            updateState(state, LCSTATE::PULSE);
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
                    if (current) ESP_LOGI(TAG, "Got new event: %s", eventToString(current.value()));
                    updateState(state, LCSTATE::IDLE);
                    break;
                }

                if (setPulseColor(ev.data.fadeColor.from, ev.data.fadeColor.to, ev.data.fadeColor.time, false))
                {
                }
            } break;

            case LCSTATE::CLEAR:
                if (setStaticColor(Color{"000000"}))
                {
                    updateState(state, LCSTATE::IDLE);
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


static constexpr const char *stateToString(const LCSTATE &state)
{
    switch (state)
    {
        case LCSTATE::IDLE: return "IDLE";
        case LCSTATE::PULSE: return "PULSE";
        case LCSTATE::CLEAR: return "CLEAR";
    }

    return "UNKNOWN STATE";
}


static constexpr const char *eventToString(const Event &ev)
{
    switch (ev.type)
    {
        case MsgType::STATIC: return "STATIC";
        case MsgType::FADE: return "FADE";
        case MsgType::PULSE: return "PULSE";
        case MsgType::NONE: return "NONE";
    }

    return "UNKNOWN EVENT";
}


static void updateState(LCSTATE &state, const LCSTATE &newState)
{
    ESP_LOGI(TAG, "[%s]->[%s]", stateToString(state), stateToString(newState));
    state = newState;
}
