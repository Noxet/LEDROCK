#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_CLK_FREQ   5000
#define LEDC_IO_RED     16
#define LEDC_IO_GREEN   17
#define LEDC_IO_BLUE    18

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
    ledc_channel_config_t led_conf = {
        .gpio_num = LEDC_IO_RED,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&led_conf);

    while (1)
    {
        printf("Setting duty = 256\n");
        ledc_set_duty(led_conf.speed_mode, led_conf.channel, 256);
        ledc_update_duty(led_conf.speed_mode, led_conf.channel);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("Setting duty = 512\n");
        ledc_set_duty(led_conf.speed_mode, led_conf.channel, 512);
        ledc_update_duty(led_conf.speed_mode, led_conf.channel);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("Setting duty = 1023\n");
        ledc_set_duty(led_conf.speed_mode, led_conf.channel, 1023);
        ledc_update_duty(led_conf.speed_mode, led_conf.channel);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
