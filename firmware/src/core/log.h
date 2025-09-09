#pragma once

#include "esp_log_level.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

#include <cstdarg>
#include <cstddef>


struct LRLock
{
	SemaphoreHandle_t m_mtx;
	LRLock(SemaphoreHandle_t mtx) : m_mtx(mtx) { xSemaphoreTake(m_mtx, portMAX_DELAY); }
	~LRLock() { xSemaphoreGive(m_mtx); }
};


class LRLog
{
public:
	LRLog(uint8_t *buffer, size_t size, esp_log_level_t espLogLevel);

private:
	void write(const uint8_t *data, size_t len);
	static int print(const char *fmt, va_list ap);

private:
	uint8_t *m_buffer;
	size_t m_capacity;
	size_t m_head;
	size_t m_len;
	SemaphoreHandle_t m_mtx;
};
