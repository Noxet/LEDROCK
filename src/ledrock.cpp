#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"


#include <list>
#include <functional>
#include <memory>

#include "Events/ButtonEvent.h"
#include "Events/TimerEvent.h"
#include "ColorManager.h"
#include "Colors/ColorMode.h"
#include "Colors/ColorUtils.h"




#define BTN GPIO_NUM_18


typedef std::list<std::function<void()>> EventHandler;
EventHandler eventHandler;


ColorManager g_colorManager;


extern "C"
{
    void app_main();
    void IRAM_ATTR gpio_isr_handler(void *arg);
    bool IRAM_ATTR timer_isr_handler(void *arg);
}


void IRAM_ATTR gpio_isr_handler(void *arg)
{
    ButtonPressedEvent ev(1337);
    /*
    * Here, we have to take the event by copy, since the variable goes out of scope
    * after this call. We also need to make the capture list mutable, due to the downcasting
    * in subsequent function calls.
    */
    eventHandler.push_back([ev]() mutable { g_colorManager.onEvent(&ev); });
}


bool IRAM_ATTR timer_isr_handler(void *arg)
{
    TimerAlarmEvent ev(1);
    eventHandler.push_back([ev]() mutable { g_colorManager.onEvent(&ev); });
    return false;
}


void app_main(void)
{


    /**
     * Enable HW fade and register interrupt.
     * Note that ledc_fade_func_install must come first, and also the SHARED flag needs to be set (docs sucks)
     */
    //ESP_ERROR_CHECK(ledc_fade_func_install(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
    //ESP_ERROR_CHECK(ledc_isr_register(led_isr_handler, NULL, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED, NULL));

    //ledc_set_fade_with_time(led_conf[RED].speed_mode, led_conf[RED].channel, 1023, 5000);
    //ledc_fade_start(led_conf[RED].speed_mode, led_conf[RED].channel, LEDC_FADE_NO_WAIT);

    

    gpio_config_t btnConfig =
    {
        .pin_bit_mask = 1ULL << BTN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&btnConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN, gpio_isr_handler, NULL);


    auto red = std::unique_ptr<ColorMode>(new StaticColor(RGB(1000, 0, 0)));
    auto yellow = std::unique_ptr<ColorMode>(new StaticColor(RGB(1000, 600, 0)));
    auto green = std::unique_ptr<ColorMode>(new StaticColor(RGB(0, 1000, 0)));

    Timer swTim(1500, timer_isr_handler, true);
    auto switchColors = std::vector<RGB>{ RGB(1000, 1000, 0), RGB(0, 1000, 1000), RGB(1000, 0, 1000) };
    auto switchColor = std::unique_ptr<ColorMode>(new SwitchingColor(switchColors, swTim));

    g_colorManager.addColorMode(move(red))
        .addColorMode(move(yellow))
        .addColorMode(move(green))
        .addColorMode(move(switchColor));
    
    g_colorManager.start();

    while (1)
    {
        while (eventHandler.empty())
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        auto &f = eventHandler.front();
        f();

        taskDISABLE_INTERRUPTS();
        eventHandler.pop_front();
        taskENABLE_INTERRUPTS();
    }
}
