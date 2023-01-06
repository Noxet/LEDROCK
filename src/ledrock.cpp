#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_CLK_FREQ 5000
#define LEDC_IO_RED 16
#define LEDC_IO_GREEN 17
#define LEDC_IO_BLUE 18

extern "C"
{
    void app_main();
}

typedef enum
{
    RED,
    GREEN,
    BLUE
} ledcolor;


volatile static int test = 0;


void set_led(ledc_channel_config_t *led_conf, int duty)
{
    ledc_set_duty(led_conf->speed_mode, led_conf->channel, duty);
    ledc_update_duty(led_conf->speed_mode, led_conf->channel);
}

void IRAM_ATTR led_isr_handler(void *arg)
{
    test = 1;
}

void app_main(void)
{
    // Set up PWM timer
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_CLK_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    // Set up LED channels
    ledc_channel_config_t led_conf[3] = {
        {
            .gpio_num = LEDC_IO_RED,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_FADE_END,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            
        },
        {
            .gpio_num = LEDC_IO_GREEN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_1,
            .intr_type = LEDC_INTR_FADE_END,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            
        },
        {
            .gpio_num = LEDC_IO_BLUE,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_2,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0
        }
    };

    for (int i = 0; i < 3; i++)
    {
        ledc_channel_config(&led_conf[i]);
    }

    /**
     * Enable HW fade and register interrupt.
     * Note that ledc_fade_func_install must come first, and also the SHARED flag needs to be set (docs sucks)
     */
    ESP_ERROR_CHECK(ledc_fade_func_install(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED));
    ESP_ERROR_CHECK(ledc_isr_register(led_isr_handler, NULL, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED, NULL));

    ledc_set_fade_with_time(led_conf[RED].speed_mode, led_conf[RED].channel, 1023, 5000);
    ledc_fade_start(led_conf[RED].speed_mode, led_conf[RED].channel, LEDC_FADE_NO_WAIT);

    while (1)
    {
        if (test) {
            printf("INTERRUPT TRIGGED!\n");
            test = 0;
        }
        /*
        printf("Setting red duty = 256\n");
        set_led(&led_conf[RED], 256);
        set_led(&led_conf[GREEN], 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("Setting green duty = 512\n");
        set_led(&led_conf[RED], 0);
        set_led(&led_conf[GREEN], 512);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("Setting g/r duty = 256\n");
        set_led(&led_conf[RED], 256);
        set_led(&led_conf[GREEN], 256);
        */
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
