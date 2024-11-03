#Pico projects: Cmake build arm none eabi
#Always required in cmake files, lower case commands preferred
cmake_minimum_required(VERSION 3.12)

#Seems to be in everyone's file, link to sdk, with build functions
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

#set PROJECT_NAME
project(Motor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Creates pico-sdk subdirectory in our project for libraries
pico_sdk_init()

#Where is main
add_executable(${PROJECT_NAME} main.c)

#create map/bin/hex/uf2 file (uf2 goes directly on the pi)
pico_add_extra_outputs(${PROJECT_NAME})

#Pull in pico_stdlib
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pwm
    hardware_clocks
)