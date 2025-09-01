#include "driver/gpio.h"
#include "drivers/ledstrip.h"
#include "hal/gpio_types.h"
#include "led_controller.h"
#include "uart_cli.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "soc/gpio_num.h"

#include <stdio.h>


extern "C" void app_main(void)
{
    Ledstrip leds;
    LedController lc{leds};
    UartCLI cli{lc};
    // lc.init();
    // lc.setStaticColor(Color{"ff0000"});
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // lc.setStaticColor(Color{"00ff00"});
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // lc.setStaticColor(Color{"ff00ff"});
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // lc.setFadeColor(Color{"000000"}, Color{"FFFFFF"}, 2000);
    // lc.setFadeColor(Color{"FF00FF"}, Color{"00FF00"}, 5000);
    // UartCLI uart(lc);


    uint8_t *data = (uint8_t *) malloc(1024);
    while (1)
    {
        //gpio_set_level(LEDPIN, 1);
        cli.poll();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        //gpio_set_level(LEDPIN, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
