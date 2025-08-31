#include "drivers/ledstrip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>


extern "C" void app_main(void)
{

    Ledstrip leds;
    leds.init();
    leds.setStaticColor(Color{"ff0000"});
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    leds.setStaticColor(Color{"00ff00"});
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    leds.setStaticColor(Color{"ff00ff"});
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    leds.setFadeColor(Color{"000000"}, Color{"FFFFFF"}, 2000);
    leds.setFadeColor(Color{"FF00FF"}, Color{"00FF00"}, 5000);
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
