#include "uart_cli.h"

#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"


UartCLI::UartCLI(LedController &controller)
    : m_controller(controller)
{
    usb_serial_jtag_driver_config_t usb_config = {
        .tx_buffer_size = 1024,
        .rx_buffer_size = 1024,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_config));
}


void UartCLI::poll()
{
    uint8_t data[256];
    int len = usb_serial_jtag_read_bytes(data, sizeof(data) - 1, 20 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        data[len] = '\0';
        printf("GOT: %s\n", data);
    }
}
