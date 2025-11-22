#include "core/log.h"
#include "core/sys.h"
#include "driver/gpio.h"
#include "drivers/ledstrip.h"
#include "http_server.h"
#include "led_controller.h"
#include "uart_cli.h"
#include "http_server.cpp"
#include "core/log.h"

#include "esp_log.h"
#include "esp_log_level.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"

#include <cstdio>


extern "C" void app_main(void)
{
    sys_init();

    LRLOGI("Booting up, post HTTP and LRLog init\n");

    Ledstrip leds;
    LedController lc{leds};
    xTaskCreate(&LedController::ledControllerTask, "lc task", 4096, &lc, 10, nullptr);

    UartCLI cli{lc.m_queue};

    HTTPServer httpServer{lc.m_queue}; // TODO(noxet): refactor to call getQueue instead
    xTaskCreate(&HTTPServer::httpServerTask, "http task", 4096, &httpServer, 5, nullptr);

    LRLog::instance().init(httpServer.getQueue());

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
