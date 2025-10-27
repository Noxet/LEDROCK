idf_component_register(SRCS
        "ledrock.cpp"
        "led_controller.cpp"
        "uart_cli.cpp"
        "http_server.cpp"
        "core/sys.cpp"
        "core/color.cpp"
        "core/log.cpp"
        "drivers/ledstrip.cpp"
        PRIV_REQUIRES spi_flash esp_driver_gpio esp_driver_ledc esp_driver_usb_serial_jtag esp_timer nvs_flash esp_http_server esp_wifi esp_eth
        INCLUDE_DIRS ".")
