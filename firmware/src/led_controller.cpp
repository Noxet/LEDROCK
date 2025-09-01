#include "led_controller.h"
#include "core/color.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


LedController::LedController(ILedDriver &driver)
    : m_driver(driver)
{

}


void LedController::init()
{
    m_driver.init();
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
