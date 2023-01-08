#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "driver/timer.h"

#include <list>
#include <functional>
#include <memory>

#include "Events/ButtonEvent.h"
#include "Events/TimerEvent.h"
#include "ColorManager.h"
#include "Colors/ColorMode.h"
#include "Colors/ColorUtils.h"


#define TIMER_INTERVAL 1500

constexpr unsigned long long TIMER_DIVIDER = 16;
constexpr unsigned long long TIMER_SCALE = TIMER_BASE_CLK / TIMER_DIVIDER / 1000;

#define BTN GPIO_NUM_18

/*
 * ISR handlers
 */
void handleBtnEvent(Event *ev);
void handleTimerEvent(Event *event);


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
    eventHandler.push_back([ev]() mutable { handleBtnEvent(&ev); });
}


void handleBtnEvent(Event *event)
{
    static int count = 0;

    if (auto ev = event_cast<ButtonPressedEvent>(event))
    {
        printf("[handleBtnEvent] - %d - Count = %d\n", ev->getTest(), ++count);

        //g_colorManager.nextColorMode();
    }
}


bool IRAM_ATTR timer_isr_handler(void *arg)
{
    TimerAlarmEvent ev(1);
    eventHandler.push_back([ev]() mutable { handleTimerEvent(&ev); });
    return false;
}


void handleTimerEvent(Event *event)
{
    if (auto ev = event_cast<TimerAlarmEvent>(event))
    {
        printf("[handleTimerEvent] - Timer ID: %d\n", ev->getTimerId());
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
        timer_set_alarm(TIMER_GROUP_0, TIMER_0, TIMER_ALARM_EN);
    }
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

    g_colorManager.addColorMode(move(red)).addColorMode(move(yellow)).addColorMode(move(green));


    timer_config_t config;
    config.alarm_en = TIMER_ALARM_EN;
    config.auto_reload = TIMER_AUTORELOAD_DIS;
    config.counter_dir = TIMER_COUNT_UP;
    config.divider = 16; // clock divider, recommend using a value between 100 and 1000
    config.intr_type = TIMER_INTR_LEVEL;
    config.counter_en = TIMER_PAUSE;
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL * TIMER_SCALE); // convert interval to microseconds
    
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_isr_handler, nullptr, 0);
    timer_start(TIMER_GROUP_0, TIMER_0);


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
