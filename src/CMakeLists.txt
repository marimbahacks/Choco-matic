add_executable(${PROJECT_NAME} 
    main.cpp
    SpeedControl.cpp
    )

#Pull in pico_stdlib
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pwm
    hardware_clocks
)    

#create map/bin/hex/uf2 file (uf2 goes directly on the pi)
pico_add_extra_outputs(${PROJECT_NAME})

#  /dev/ttyACM0 with 115200 poll rate for USB on putty
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)