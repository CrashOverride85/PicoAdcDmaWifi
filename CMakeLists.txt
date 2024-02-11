# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

set(PICO_BOARD pico_w)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(DmaAdcBluethooth C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(DmaAdcBluethooth 
  DmaAdcBluethooth.cpp
  CAnalogueCapture.cpp 
)

pico_set_program_name(DmaAdcBluethooth "DmaAdcBluethooth")
pico_set_program_version(DmaAdcBluethooth "0.1")

pico_enable_stdio_uart(DmaAdcBluethooth 1)
pico_enable_stdio_usb(DmaAdcBluethooth 0)

# Add the standard library to the build
target_link_libraries(DmaAdcBluethooth
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(DmaAdcBluethooth PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(DmaAdcBluethooth 
        hardware_dma
        hardware_adc
        pico_cyw43_arch_lwip_poll
        pico_btstack_cyw43
        pico_btstack_ble
        )

pico_add_extra_outputs(DmaAdcBluethooth)


target_compile_definitions(DmaAdcBluethooth PRIVATE
# PICO_STACK_SIZE=8192
# PICO_CORE1_STACK_SIZE=0
# PICO_USE_STACK_GUARDS
# PICO_MALLOC_PANIC=0
)
