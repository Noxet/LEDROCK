idf_component_register(SRCS "ledrock.cpp" "led_controller.cpp" "uart_cli.cpp" "core/color.cpp" "drivers/ledstrip.cpp"
        PRIV_REQUIRES spi_flash esp_driver_gpio esp_driver_ledc esp_driver_usb_serial_jtag
        INCLUDE_DIRS ".")
