#include "uart_cli.h"

#include "core/color.h"
#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <cstring>


UartCLI::UartCLI(QueueHandle_t &lcQueue)
    : m_lcQueue(lcQueue)
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


/**
 * Protocol:
 * |action|
 */
void UartCLI::parse()
{
    printf("parse\n");
    // TODO: log failed posts to queue
    if (xQueueSend(m_lcQueue, &m_buffer[0], 10) != pdPASS)
    {
        printf("UART Failed to send data\n");
    }
    printf("sent to queue\n");
//     m_bufferToParse = false;
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
