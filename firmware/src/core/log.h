#pragma once

#include "esp_log_level.h"
#include "freertos/idf_additions.h"


#define LRLOG_TAG __FILE__

class LRLog
{
public:
	static LRLog &instance();
	void init(QueueHandle_t httpQueue);
	void log(esp_log_level_t level, const char *tag, const char *fmt, ...);

private:
	LRLog() = default;

	QueueHandle_t m_httpQueue = nullptr;
};

#define LRLOGI(fmt, ...) LRLog::instance().log(ESP_LOG_INFO, LRLOG_TAG, fmt, ##__VA_ARGS__);
