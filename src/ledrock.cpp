#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "soc/clk_tree_defs.h"

#include <atomic>
#include <stdio.h>
#include <inttypes.h>

constexpr gpio_num_t LEDPIN = GPIO_NUM_16;

std::atomic<int> count = 0;

static  bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    BaseType_t taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT)
    {
        SemaphoreHandle_t sem = (SemaphoreHandle_t) user_arg;
        xSemaphoreGiveFromISR(sem, &taskAwoken);
    }

    return taskAwoken;
}

extern "C" void app_main(void)
{
    gpio_config_t io_conf{};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << LEDPIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //gpio_config(&io_conf);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = 0,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = 16,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = { .output_invert = 0 },
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);
    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event,
    };

    SemaphoreHandle_t ledSem = xSemaphoreCreateCounting(1, 1);

    ledc_cb_register(ledc_channel.speed_mode, ledc_channel.channel, &callbacks, ledSem);
    xSemaphoreTake(ledSem, portMAX_DELAY);
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 8000, 2000, LEDC_FADE_NO_WAIT);

    xSemaphoreTake(ledSem, portMAX_DELAY);
    printf("start fading down...\n");
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 0, 2000, LEDC_FADE_NO_WAIT);

    while (1)
    {
        //gpio_set_level(LEDPIN, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        //gpio_set_level(LEDPIN, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
