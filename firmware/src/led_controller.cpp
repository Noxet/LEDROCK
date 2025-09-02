#include "led_controller.h"
#include "core/color.h"

#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "portmacro.h"
#include <cstdint>


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
    uint8_t data;
    while (1)
    {
        if (xQueueReceive(m_queue, &data, 10) == pdPASS)
        {
            printf("LC got data\n");

            switch (data)
            {
                case '0':
                    setStaticColor(Color{"000000"});
                    break;
                case '1':
                    setStaticColor(Color{"FF0000"});
                    break;
                case '2':
                    setStaticColor(Color{"00FF00"});
                    break;
                case '3':
                    setStaticColor(Color{"0000FF"});
                    break;
                case '4':
                    setStaticColor(Color{"FFFFFF"});
                    break;
                case '5':
                    setStaticColor(Color{255, 182, 78});
                    break;
                case '6':
                    setStaticColor(Color{"FFD400"});
                    break;
                case '7':
                    setFadeColor(Color{"FF12A0"}, Color{"A0BD45"}, 3000);
                    break;
            }
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
}


/**
 * Just grab the led controller from the passed param, and run the inf loop
 */
void LedController::ledControllerTask(void *pvParam)
{
    LedController *lc = static_cast<LedController *>(pvParam);
    lc->run();
}

