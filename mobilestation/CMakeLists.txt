cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(mobilestation)

target_include_directories(app PRIVATE inc)

# python3 /root/ncs/bootloader/mcuboot/scripts/imgtool.py keygen -t ecdsa-p256 -k wlabapp.pem
set(mcuboot_CONFIG_BOOT_SIGNATURE_KEY_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/wlabapp.pem\")

target_sources(app PRIVATE 
    src/periphery/uart_gsm.c
    src/periphery/gpio_sim800l.c
    src/periphery/gpio_ext3v3.c
    src/periphery/gpio_status_led.c
    src/periphery/gpio_user_btn.c
    src/periphery/adc_battery_vol.c
    src/main.c
    src/nvs_data.c
    src/shell_commands.c
    src/sim800l.c
    src/wlab.c
    src/ble_worker.c
)
