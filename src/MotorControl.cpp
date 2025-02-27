#include "MotorControl.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

MotorControl::MotorControl()
   //To implement: percentage out of 6249
   //:m_desired_speed{desired_speed},
   :lift_up_gpio {2},
   lower_down_gpio {16},
   debounce_timer {50},
   pwm_control_gpio {0},
   pwm_reverse_control_gpio {14},
   period {6250},
   time {to_ms_since_boot(get_absolute_time())}

{
}

void MotorControl::check_button(){
    if (!gpio_get(lift_up_gpio)){
        set_rotation(lift_up_gpio);
    }
    else if (!gpio_get(lower_down_gpio)){
        set_rotation(lower_down_gpio);
    }
    else {
        set_rotation(-1);
    }
}
void MotorControl::pwm_pin_setup(){
    pwm_pin_setup(pwm_control_gpio);
    pwm_pin_setup(pwm_reverse_control_gpio);
}


void MotorControl::pwm_pin_setup(uint control_pin){
    //Set PWM functionality with default of 0
    gpio_set_function(control_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(control_pin, 0);

    //Find hardware PWM information
    uint slice_num = pwm_gpio_to_slice_num(control_pin);
    uint channel = pwm_gpio_to_channel(control_pin);

    //PWM Adjustment Levers (12V DC Motor estimated start: 20kHz instead of 50Hz servo
    //period is now class member, can be adjusted here if necessary
    period = period;
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
    pwm_set_chan_level(slice_num, channel, period - 1);

    //Initialize the slice found earlier
    pwm_init(slice_num, &config, false);
    // Enable PWM after setup is complete
    pwm_set_enabled(slice_num, true);
    printf("PWM Pin %d is enabled\n", control_pin);
}


void MotorControl::button_setup(){
    button_setup(lift_up_gpio, lower_down_gpio);
}

void MotorControl::button_setup(uint lift_button_pin, uint lower_button_pin){
    //Prep Button GPIOs
    gpio_init(lift_button_pin);
    gpio_init(lower_button_pin);

    //Directionally pressing down makes the action
    gpio_set_dir(lift_button_pin, GPIO_IN);
    gpio_set_dir(lower_button_pin, GPIO_IN);

    gpio_pull_up(lift_button_pin);
    gpio_pull_up(lower_button_pin);
    
}

void MotorControl::controller_enable_pin_setup(){
    controller_enable_pin_setup(pwm_control_gpio);
    controller_enable_pin_setup(pwm_reverse_control_gpio);
}

void MotorControl::controller_enable_pin_setup(uint enable){
    gpio_init(enable);
    gpio_set_dir(enable, GPIO_OUT);
    gpio_put(enable, 1);

}
void MotorControl::set_rotation(int pin_pressed){
    //Prio given to lifting up over lowering
    if (pin_pressed == lift_up_gpio){
        printf("Lift up pin is pressed\n");
        //Max speed forward
        pwm_set_gpio_level(pwm_control_gpio, period - 1);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 0);
    }
    //First PWM pin backward, 2nd forward
    else if (pin_pressed == lower_down_gpio){
        printf("Lower down pin is pressed\n");
        //Max speed backward
        pwm_set_gpio_level(pwm_control_gpio, 0);
        pwm_set_gpio_level(pwm_reverse_control_gpio, period - 1);
    }
    else {
         //In all other scenarios, neutral motor speed/position
        printf("No control pin pressed \n");
        pwm_set_gpio_level(pwm_control_gpio, 0);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 0);
    }
}

bool MotorControl::debounce_check(uint32_t previous_time, uint pin){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - previous_time > debounce_timer){
        return true;
    }
    return false;
}