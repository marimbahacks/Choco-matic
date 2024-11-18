#include "MotorControl.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

MotorControl::MotorControl(int desired_speed)
   //To implement: percentage out of 6249
   :m_desired_speed{desired_speed}
{
}

void MotorControl::pwm_pin_setup(uint control_pin){
    //Set PWM functionality with default of 0
    gpio_set_function(control_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(control_pin, 0);

    //Find hardware PWM information
    uint slice_num = pwm_gpio_to_slice_num(control_pin);
    uint channel = pwm_gpio_to_channel(control_pin);

    //PWM Adjustment Levers (12V DC Motor estimated start: 20kHz instead of 50Hz servo
    uint period = 6250;
    uint desired_output_hz = 20000;

    //Get base of system clock (should be 125MHz)
    uint32_t clock = clock_get_hz(clk_sys);
    //Clock divider needed as lowest PWM freq of pi pico is 1.9kHz without
    // div = clock (125MHz) / period of 6250 * freq desired (20000) makes clk div = 1
    uint32_t clk_div = clock / (period * desired_output_hz);
    printf("Clock divider is %d for PWM Pin %d\n", clk_div, control_pin);
    //Clock division is 4 bit unsigned int (therefore 255 max)
    //and sent in as float, therefore clock_divide can never be <1 or >255
    if (clk_div < 1){
        printf("Divider was %d, setting to 1\n", clk_div);
        clk_div = 1;
    }

    if (clk_div > 255){
        printf("Divider was %d, setting to 255\n", clk_div);
        clk_div = 255;
        
    }
    //Spencer: start making printf into debug only builds?
    printf("Clock Divider for Pin %d is %d\n", control_pin, clk_div);

    pwm_config config = pwm_get_default_config();
    //Change default config to include divider
    pwm_config_set_clkdiv(&config, (float)clk_div);

    //Set wrap number
    pwm_config_set_wrap(&config, period);
    //Spencer: speed adjustment percentage here or in separate method?
    pwm_set_chan_level(slice_num, channel, 6249);

    //Initialize the slice found earlier
    pwm_init(slice_num, &config, false);
    // Enable PWM after setup is complete
    pwm_set_enabled(slice_num, true);
    printf("PWM Pin %d is enabled\n", control_pin);
}


void MotorControl::set_rotation(int pin_pressed, enum gpio_irq_level button_state){

    
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