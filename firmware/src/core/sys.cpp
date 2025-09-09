#include "sys.h"

#include "esp_timer.h"


const char *LRTAG = "LR";

uint64_t getUptime()
{
    return esp_timer_get_time() / 1000;
}
