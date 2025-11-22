#pragma once

#include "esp_http_server.h"
#include "freertos/idf_additions.h"


class HTTPServer
{
public:
	HTTPServer(QueueHandle_t &lcQueue);
	static void httpServerTask(void *pvParam);

	void startWebserver();
	void stopWebserver();
	httpd_handle_t getServer();
	QueueHandle_t getQueue();

private:
	void init();
	void run();

	QueueHandle_t m_httpQueue;
	httpd_handle_t m_server;

	QueueHandle_t &m_lcQueue; // ref to the LedController event queue
};
