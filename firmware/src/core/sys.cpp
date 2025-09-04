#include "sys.h"

#include "esp_timer.h"

uint64_t getUptime()
{
    return esp_timer_get_time() / 1000;
}
