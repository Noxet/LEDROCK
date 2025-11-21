# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "src")


# Gzip and embed the html files for the webpage
set(SRC_HTML ${CMAKE_SOURCE_DIR}/html/index.html)
set(GZ_HTML ${CMAKE_BINARY_DIR}/index.html.gz)

message("Gzipping index.html...")
execute_process(
        COMMAND gzip -c ${SRC_HTML}
        OUTPUT_FILE ${GZ_HTML}
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ledrock)
