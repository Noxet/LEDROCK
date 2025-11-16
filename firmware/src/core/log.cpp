#include "log.h"
#include "core/sys.h"
#include "http_server.h"

#include "esp_log.h"
#include "esp_log_level.h"
#include <cstdio>
#include <cstring>


// Store the ring buffer in LP RAM (16kiB). There are some variables reserved here,
// so we use a little less than the full 16kiB.
RTC_DATA_ATTR static uint8_t s_ringBuffer[16000];


LRLog &LRLog::instance()
{
	static LRLog log;
	return log;
}


void LRLog::init(QueueHandle_t httpQueue)
{
	printf("INIT this=%p\n", this);
	m_httpQueue = httpQueue;
}


void LRLog::log(esp_log_level_t level, const char *tag, const char *fmt, ...)
{
	printf("LOG this=%p, httpQueue=%p\n", this, m_httpQueue);
	char msg[256];

	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	if (!len) return;
	// if test got truncated, adjust len accordingly
	if (len >= sizeof(msg)) len = sizeof(msg) - 1;

	esp_log_write(level, tag, "%s\n", msg);

	if (m_httpQueue)
	{
		char *copy = static_cast<char *>(malloc(len + 1));
		memcpy(copy, msg, len + 1);
		xQueueSendToBack(m_httpQueue, &copy, 0);
	}
}
