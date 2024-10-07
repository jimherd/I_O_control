<<<<<<< HEAD
# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CMakePrintHelpers)

set (PROJECT_NAME "Pi_Robot_test")

set(PICO_BOARD pico CACHE STRING "Board type")

# Set PICO_SDK_PATH to point to appropriate sdk direcory
    set(ENV{PICO_SDK_PATH} "${USERHOME}/.pico_sdk")
    cmake_print_variables(PICO_SDK_PATH)
 
# set FreeRTOS directory  
    cmake_path( SET FREERTOS_KERNEL_DIR "${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel")   # local
    cmake_print_variables(FREERTOS_KERNEL_DIR)


#  Some basic includes
    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    include(${PICO_SDK_PATH}/tools/CMakeLists.txt)
    include(${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Set languages, compiler options
    project(${PROJECT_NAME} C CXX ASM)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fshort-enums -Wunused-result ")    # -fshort-enums

# initialize the Pico SDK
    pico_sdk_init()

# collect all source .c files and add to project
    FILE(GLOB Pico_source_files ${CMAKE_SOURCE_DIR}/src/*.c)

    add_executable(${PROJECT_NAME}
        ${Pico_source_files}
    )

# Get Freertos source files
FILE(GLOB FreeRTOS_src ${FREERTOS_KERNEL_DIR}/*.c)

add_library( FreeRTOS STATIC
    ${FreeRTOS_src}
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM0/port.c
    ${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c
    )

target_include_directories( FreeRTOS PUBLIC
    ${FREERTOS_KERNEL_DIR}/include
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM0
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories ( ${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries( ${PROJECT_NAME} 
    pico_stdlib 
    pico_multicore
    hardware_gpio 
    hardware_i2c
    hardware_spi
    hardware_pwm
    hardware_adc
    hardware_pio
    hardware_dma
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    )

pico_add_extra_outputs(${PROJECT_NAME})

# Disable uart/usb serial channels
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# print executable size
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND date.exe /t >> ${CMAKE_SOURCE_DIR}/File_size.txt
    COMMAND ${USERHOME}/.pico-sdk/toolchain/13_2_Rel1/bin/arm-none-eabi-size.exe -G ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}.elf
    COMMAND ${USERHOME}/.pico-sdk/toolchain/13_2_Rel1/bin/arm-none-eabi-size.exe -G ${CMAKE_SOURCE_DIR}/build/${PROJECT_NAME}.elf  >> ${CMAKE_SOURCE_DIR}/File_size.txt
    VERBATIM
)


# file(TO_CMAKE_PATH "$ENV{PICO_SDK_PATH}" PICO_SDK_PATH_CMAKE)   ** unused

# initialize the SDK based on PICO_SDK_PATH
# include(pico_sdk_import.cmake)

# include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# generate the header file into the source tree as it is included in the RP2040 datasheet
# pico_generate_pio_header(${PROJECT_NAME} 
#   ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
=======
cmake_minimum_required(VERSION 3.13)

include(CMakePrintHelpers)

set (PROJECT_NAME "Pi_the_Robot")  # set project name

# cmake_path( SET FREERTOS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel")
cmake_path (SET FREERTOS_SOURCE_DIR "$ENV{PICO_SDK_PATH}/lib/FreeRTOS-Kernel")


cmake_print_variables(FREERTOS_SOURCE_DIR)

file(TO_CMAKE_PATH "$ENV{PICO_SDK_PATH}" PICO_SDK_PATH_CMAKE)

# initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

# include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

# project(Pico_FreeRTOS_template C CXX ASM)
project(${PROJECT_NAME} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Pico SDK
pico_sdk_init()

# add gcc compiler options

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fshort-enums -Wunused-result ")    # -fshort-enums

# collect all source .c files
FILE(GLOB Pico_source_files ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(${PROJECT_NAME}
    ${Pico_source_files}
)

# generate the header file into the source tree as it is included in the RP2040 datasheet
# pico_generate_pio_header(${PROJECT_NAME} 
#   ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

FILE(GLOB FreeRTOS_src ${FREERTOS_SOURCE_DIR}/*.c)   # CMAKE_SOURCE_DIR

add_library( FreeRTOS STATIC
    ${FreeRTOS_src}
    ${FREERTOS_SOURCE_DIR}/portable/GCC/ARM_CM0/port.c
    ${FREERTOS_SOURCE_DIR}/portable/MemMang/heap_4.c
    )

target_include_directories( FreeRTOS PUBLIC
    ${FREERTOS_SOURCE_DIR}/include
    ${FREERTOS_SOURCE_DIR}/portable/GCC/ARM_CM0
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories ( ${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries( ${PROJECT_NAME} 
    pico_stdlib 
    pico_multicore
    hardware_gpio 
    hardware_i2c
    hardware_spi
    hardware_pwm
    hardware_adc
    hardware_pio
    hardware_dma
    FreeRTOS
    )

pico_add_extra_outputs(${PROJECT_NAME})

# Disable uart/usb serial channels
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# print executable size
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND date.exe /t >> ${CMAKE_SOURCE_DIR}/File_size.txt
    COMMAND arm-none-eabi-size.exe -G ${PROJECT_NAME}.elf
    COMMAND arm-none-eabi-size.exe -G ${PROJECT_NAME}.elf  >> ${CMAKE_SOURCE_DIR}/File_size.txt
    VERBATIM
)

  #  ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c
  #  ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/MemMang/heap_4.c

  #  ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel/include
  #  ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Kernel/portable/GCC/ARM_CM0
  #  ${CMAKE_SOURCE_DIR}/include
>>>>>>> 8c66029fb43c52ad1e392ea3580787dda24bd318
