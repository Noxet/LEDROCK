#include "sys.h"

#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_log.h"


void sys_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

uint64_t sys_getUptime()
{
    return esp_timer_get_time() / 1000;
}
