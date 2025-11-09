cmake_minimum_required(VERSION 3.13)

project(midi_keyboard C CXX ASM)

set(PICO_SDK_PATH "/home/willem/midi-keyboard-pico/pico/pico-sdk")

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
pico_sdk_init()

add_executable(midi_keyboard
    main.c
    src/usb_midi.c
)

target_include_directories(midi_keyboard PRIVATE include)

target_link_libraries(midi_keyboard
    pico_stdlib
    tinyusb_device
    tinyusb_boardc
)

pico_enable_stdio_usb(midi_keyboard 1)
pico_enable_stdio_uart(midi_keyboard 0)

pico_add_extra_outputs(midi_keyboard)
