#include "core/log.h"
#include "core/sys.h"
#include "driver/gpio.h"
#include "drivers/ledstrip.h"
#include "led_controller.h"
#include "uart_cli.h"

#include "esp_log.h"
#include "esp_log_level.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"

#include <cstdio>


extern "C" void app_main(void)
{
    Ledstrip leds;
    LedController lc{leds};
    lc.init();
    UartCLI cli{lc.m_queue};
    // UartCLI uart(lc);

    xTaskCreate(&LedController::ledControllerTask, "lc task", 4096, &lc, 10, nullptr);


    while (1)
    {
        //gpio_set_level(LEDPIN, 1);
        cli.poll();
        vTaskDelay(10 / portTICK_PERIOD_MS);
        //gpio_set_level(LEDPIN, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
