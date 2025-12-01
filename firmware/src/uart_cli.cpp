#include "uart_cli.h"
#include "message.h"
#include "core/color.h"

#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"

#include <cstring>


static const char *TAG = "UART";


UartCLI::UartCLI(QueueHandle_t lcQueue)
    : m_lcQueue(lcQueue)
{
    usb_serial_jtag_driver_config_t usb_config = {
        .tx_buffer_size = 1024,
        .rx_buffer_size = 128,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_config));

    m_bufferDataSize = 0;
    m_bufferReadPos = m_buffer;
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
    logPacket();
    // TODO: log failed posts to queue
    // if (m_bufferPos < 3) return;
    // discard the size byte
    read_u8_le();
    MsgType type = static_cast<MsgType>(read_u8_le());
    Event e;
    e.type = type;
    switch (type)
    {
        case MsgType::NONE:
            break;
        case MsgType::STATIC:
            {
                e.data.staticColor.color.r = read_u8_le();
                e.data.staticColor.color.g = read_u8_le();
                e.data.staticColor.color.b = read_u8_le();
            }
            break;
        case MsgType::FADE:
            /* Fall-through */
        case MsgType::PULSE:
            {
                e.data.fadeColor.from.r = read_u8_le();
                e.data.fadeColor.from.g = read_u8_le();
                e.data.fadeColor.from.b = read_u8_le();
                e.data.fadeColor.to.r = read_u8_le();
                e.data.fadeColor.to.g = read_u8_le();
                e.data.fadeColor.to.b = read_u8_le();
                e.data.fadeColor.time = read_u32_le();
            }
            break;
    }

    ESP_LOGW(TAG, "LC queue utilization: %u/%u", uxQueueMessagesWaiting(m_lcQueue), uxQueueSpacesAvailable(m_lcQueue));
    if (xQueueSend(m_lcQueue, &e, 10) != pdPASS)
    {
        ESP_LOGW(TAG, "Failed to post to queue");
    }

}


void UartCLI::poll()
{
    // Wait for main thread to parse the latest data
    // if (m_bufferToParse) return;

    // The first byte indicates the number of bytes in the packet (minus the first byte)
    uint8_t dataLen = 0;
    int len = usb_serial_jtag_read_bytes(&dataLen, 1, 20 / portTICK_PERIOD_MS);
    if (!len) return;
    m_buffer[0] = dataLen;
    int bytesRead = 0;
    while (bytesRead < dataLen)
    {
        bytesRead += usb_serial_jtag_read_bytes(&m_buffer[bytesRead + 1], dataLen - bytesRead, 20 / portTICK_PERIOD_MS);
    }

    m_bufferDataSize = bytesRead + 1;
    parse();
    memset(m_buffer, '\0', sizeof(m_buffer));
    m_bufferReadPos = m_buffer;
    /*
    int len = usb_serial_jtag_read_bytes(&m_buffer[m_bufferDataSize], 1, 10 / portTICK_PERIOD_MS);
    m_bufferDataSize += len;
    if (isLineEnding(m_buffer[m_bufferDataSize - 1]))
    {
        m_buffer[m_bufferDataSize - 1] = '\0';
        parse();
        memset(m_buffer, '\0', sizeof(m_buffer));
        m_bufferDataSize = 0;
        m_bufferReadPos = m_buffer;
        m_bufferToParse = true;
    }
    */

}


uint8_t UartCLI::read_u8_le()
{
    uint8_t val = *m_bufferReadPos++;
    return val;
}


uint16_t UartCLI::read_u16_le()
{
    uint16_t val = 0;
    val |= static_cast<uint16_t>(*m_bufferReadPos++) << 0;
    val |= static_cast<uint16_t>(*m_bufferReadPos++) << 8;
    return val;
}


uint32_t UartCLI::read_u32_le()
{
    uint32_t val = 0;
    val |= static_cast<uint32_t>(*m_bufferReadPos++) << 0;
    val |= static_cast<uint32_t>(*m_bufferReadPos++) << 8;
    val |= static_cast<uint32_t>(*m_bufferReadPos++) << 16;
    val |= static_cast<uint32_t>(*m_bufferReadPos++) << 24;
    return val;
}

void UartCLI::logPacket()
{
    char buf[2 * sizeof(m_buffer)];
    int pos = 0;
    for (int i = 0; i < m_bufferDataSize; i++)
    {
        pos += snprintf(&buf[pos], sizeof(buf), "%02X", m_buffer[i]);
    }

    ESP_LOGI(TAG, "RX packet: %s", buf);
}
