#include "log.h"
#include "core/sys.h"

#include "esp_log.h"
#include "esp_log_level.h"
#include <cstdio>
#include <cstring>


// Store the ring buffer in LP RAM (16kiB). There are some variables reserved here,
// so we use a little less than the full 16kiB.
RTC_DATA_ATTR static uint8_t s_ringBuffer[16000];

static LRLog s_lrLog(s_ringBuffer, sizeof(s_ringBuffer), ESP_LOG_INFO);


LRLog::LRLog(uint8_t *buffer, size_t size, esp_log_level_t espLogLevel)
	: m_buffer{buffer}, m_capacity{size}, m_head{0}, m_len{0}
{
	m_mtx = xSemaphoreCreateMutex();

	// Let the ESP_LOGx call our own function
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_set_vprintf(&LRLog::print);
}


void LRLog::write(const uint8_t *data, size_t len)
{
	if (!len) return;
	
	LRLock _{m_mtx};
	if (len >= m_capacity)
	{
		// only write the tail part of the message
		std::memcpy(m_buffer, data + (len - m_capacity), m_capacity);
		m_head = 0;
		m_len = m_capacity;
		return;
	}
}


/*
 * Gets called by ESP_LOGx functions.
 * Prints to both UART and logs it in the ring buffer for WiFi.
 */
int LRLog::print(const char *fmt, va_list ap)
{
	va_list apUart;
	va_copy(apUart, ap);
	int uartLen = vprintf(fmt, apUart);
	va_end(apUart);

	va_list  apWifi;
	va_copy(apWifi, ap);
	char tmp[256];
	int wifiLen = vsnprintf(tmp, sizeof(tmp), fmt, apWifi);
	if (wifiLen > 0)
	{
		// check if print was truncated
		int toWrite = wifiLen >= sizeof(tmp) ? sizeof(tmp) : wifiLen;
		// the safe way to cast this. This will catch unexpected changes to the data type for tmp.
		s_lrLog.write(reinterpret_cast<uint8_t *>(static_cast<char *>(tmp)), toWrite);
	}
	

	return uartLen;
}
