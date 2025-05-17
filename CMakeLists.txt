

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Embarcatech_F2T3_webserver C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Embarcatech_F2T3_webserver Embarcatech_F2T3_webserver.c lib/ssd1306.c)

pico_set_program_name(Embarcatech_F2T3_webserver "Embarcatech_F2T3_webserver")
pico_set_program_version(Embarcatech_F2T3_webserver "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Embarcatech_F2T3_webserver 0)
pico_enable_stdio_usb(Embarcatech_F2T3_webserver 1)

# Add the standard library to the build
target_link_libraries(Embarcatech_F2T3_webserver
        pico_stdlib
        hardware_gpio
        hardware_pwm
        hardware_adc
        hardware_i2c
        pico_cyw43_arch_lwip_threadsafe_background
)

# Add the standard include files to the build
target_include_directories(Embarcatech_F2T3_webserver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

target_sources(Embarcatech_F2T3_webserver PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)


# Add any user requested libraries

pico_add_extra_outputs(Embarcatech_F2T3_webserver)
