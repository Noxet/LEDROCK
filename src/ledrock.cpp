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


extern "C"
{
    void app_main();
    void IRAM_ATTR gpio_isr_handler(void *arg);
    bool IRAM_ATTR timer_isr_handler(void *arg);
    bool IRAM_ATTR debounce_timer_handler(void *arg);
}


typedef std::list<std::function<void()>> EventHandler;
EventHandler eventHandler;

ColorManager g_colorManager;

Timer g_debounceTimer(20, debounce_timer_handler, false, TIMER_0);
static bool g_debounceTimerRunning { false };



void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // timer already in progress, wait until finished before starting another one
    if (g_debounceTimerRunning) return;
    
    g_debounceTimerRunning = true;
    g_debounceTimer.restart();
}


bool IRAM_ATTR debounce_timer_handler(void *arg)
{
    // timer has stopped and generated called this function
    g_debounceTimerRunning = false;

    // Button has pull-up, so it is pressed when value = 0
    if (gpio_get_level(BTN) == 1) return false;

    // detected button press after debouncing
    ButtonPressedEvent ev(0);

    /*
    * Here, we have to take the event by copy, since the variable goes out of scope
    * after this call. We also need to make the capture list mutable, due to the downcasting
    * in subsequent function calls.
    */
    eventHandler.push_back([ev]() mutable { g_colorManager.onEvent(&ev); });
    return false;
}


bool IRAM_ATTR timer_isr_handler(void *arg)
{
    TimerAlarmEvent ev(1);
    eventHandler.push_back([ev]() mutable { g_colorManager.onEvent(&ev); });
    return false;
}


bool IRAM_ATTR led_fade_isr_handler(const ledc_cb_param_t *param, void *arg)
{
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

    //ledc_set_fade_with_time(m_ledConf[RED].speed_mode, m_ledConf[RED].channel, 1023, 5000);
    //ledc_fade_start(m_ledConf[RED].speed_mode, m_ledConf[RED].channel, LEDC_FADE_NO_WAIT);

    

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

    // TODO: initialize in constructor instead? RAII
    g_debounceTimer.init();


    auto red = std::unique_ptr<ColorMode>(new StaticColor(RGB(1000, 0, 0)));
    auto yellow = std::unique_ptr<ColorMode>(new StaticColor(RGB(1000, 600, 0)));
    auto green = std::unique_ptr<ColorMode>(new StaticColor(RGB(0, 1000, 0)));

    Timer swTim(1500, timer_isr_handler, true, TIMER_1);
    auto switchColors = std::vector<RGB>{ RGB(1000, 1000, 0), RGB(0, 1000, 1000), RGB(1000, 0, 1000) };
    auto switchColor = std::unique_ptr<ColorMode>(new SwitchingColor(switchColors, swTim));

    auto fadeColor = std::unique_ptr<ColorMode>(new FadingColor(RGB(0, 0, 0), RGB(1000, 1000, 1000), swTim));

    g_colorManager.addColorMode(move(red))
        .addColorMode(move(fadeColor));
        //.addColorMode(move(yellow))
        //.addColorMode(move(green))
        //.addColorMode(move(switchColor));
    
    g_colorManager.start();

    while (true)
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
