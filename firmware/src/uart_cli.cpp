#include "uart_cli.h"

#include "core/color.h"
#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include <cstring>


UartCLI::UartCLI(LedController &controller)
    : m_controller(controller)
{
    usb_serial_jtag_driver_config_t usb_config = {
        .tx_buffer_size = 1024,
        .rx_buffer_size = 1024,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_config));

    m_bufferPos = 0;
    m_bufferToParse = false;
}


bool UartCLI::hasPacket()
{
    return false;
}


void UartCLI::parse()
{
    switch (m_buffer[0])
    {
        case '0':
            m_controller.setStaticColor(Color{"000000"});
            break;
        case '1':
            m_controller.setStaticColor(Color{"FF0000"});
            break;
        case '2':
            m_controller.setStaticColor(Color{"00FF00"});
            break;
        case '3':
            m_controller.setStaticColor(Color{"0000FF"});
            break;
        case '4':
            m_controller.setStaticColor(Color{"FFFFFF"});
            break;
        case '5':
            m_controller.setStaticColor(Color{255, 182, 78});
            break;
        case '6':
            m_controller.setStaticColor(Color{"FFD400"});
            break;
    }
    m_bufferToParse = false;
}


void UartCLI::poll()
{
    // Wait for main thread to parse the latest data
    // if (m_bufferToParse) return;

    int len = usb_serial_jtag_read_bytes(&m_buffer[m_bufferPos], sizeof(m_buffer) - 1, 20 / portTICK_PERIOD_MS);
    m_bufferPos += len;
    if (isLineEnding(m_buffer[m_bufferPos - 1]))
    {
        m_buffer[m_bufferPos - 1] = '\0';
        parse();
        memset(m_buffer, '\0', sizeof(m_buffer));
        m_bufferPos = 0;
        m_bufferToParse = true;
    }

}
