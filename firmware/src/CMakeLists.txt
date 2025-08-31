idf_component_register(SRCS "ledrock.cpp" "color_mode.cpp" "core/color.cpp" "drivers/ledstrip.cpp"
                    PRIV_REQUIRES spi_flash esp_driver_gpio esp_driver_ledc
                    INCLUDE_DIRS ".")
