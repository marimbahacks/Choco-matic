#include "speed_control.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

void set_rotation(int pin_pressed, enum gpio_irq_level button_state){

    
    //6250 period, let's do half at first?
    if (interrupt_state == GPIO_IRQ_EDGE_FALL){
        //90 should be neutral state
        pwm_set_gpio_level(pin_pressed, degree_conversion(90));
    }
    //If green button is pressed down, move at full speed clockwise (up)
    if (pin_pressed == LIFT_UP_GPIO){
        //Max speed forward
        pwm_set_gpio_level(pin_pressed, degree_conversion(180));
    }   
    //If red button is pressed down & green is not pressed, move full speed counter-clockwise (down)
    else if (pin_pressed == LOWER_DOWN_GPIO){
        //Max speed backward
        pwm_set_gpio_level(pin_pressed, degree_conversion(0));

    }
    //In all other scenarios, neutral motor speed/position
    printf("No control pin pressed \n");
    pwm_set_gpio_level(pin_pressed, degree_conversion(90));
    //I have a bone to pick with MISRA not allowing multiple return statements
}