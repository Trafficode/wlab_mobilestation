cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(mobilestation)

target_include_directories(app PRIVATE inc)

target_sources(app PRIVATE 
    src/hal/uart_gsm.c
    src/hal/gpio_sim800l.c
    src/main.c
    src/nvs_data.c
    src/shell_commands.c
    src/sim800l.c
    src/wlab.c
)
