#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

#define ROTATE_MAX 2500
#define ROTATE_MIN 500

#define PWM_CONTROL_GPIO 0
#define LIFT_UP_GPIO 2
#define LOWER_DOWN_GPIO 16


uint check_button(int pull_up_status, int drop_down_status, uint lift_speed, uint lower_speed){
    //If green button is pressed down, move at full speed clockwise (up)
    if (!gpio_get(pull_up_status)){
        return lift_speed;
    }   
    //If red button is pressed down & green is not pressed, move full speed counter-clockwise (down)
    else if (!gpio_get(drop_down_status)){
        return lower_speed;   
    }
    //In all other scenarios, neutral motor speed/position
    return 90;
    
    //I have a bone to pick with MISRA not allowing multiple return statements
}

//Move to interrupts from constant looping on button checks
void check_button_callback(uint control_pin, uint32_t events){
    printf("Interupt triggered on GPIO %d with event %d \n", control_pin, events);


}

void pwm_pin_setup(uint control_pin){
    //Set PWM functionality with default of 0
    gpio_set_function(control_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(control_pin, 0);

    //Find hardware PWM information
    uint slice_num = pwm_gpio_to_slice_num(control_pin);
    uint channel = pwm_gpio_to_channel(control_pin);

    //PWM Adjustment Levers (Servos typically are 50hz w/20s periods holdover
    //from pulse-position-modulation?)
    uint period = 20000;
    uint desired_output_hz = 50;

    //Get base of system clock (should be 125MHz)
    uint32_t clock = clock_get_hz(clk_sys);
    //Clock divider needed as lowest PWM freq of pi pico is 1.9kHz without
    // div = clock (125MHz) / (normal period of 50Hz cycle is 20s, 20000 ms) * freq desired (50Hz)
    uint32_t clk_div = clock / (period * desired_output_hz);

    //Clock division is 4 bit unsigned int (therefore 255 max)
    //and sent in as float, therefore clock_divide can never be <1 or >255
    if (clk_div < 1){
        clk_div = 1;
        //TO DO: add some logging here
    }
    if (clk_div > 255){
        clk_div = 255;
        //TO DO: add some logging here
    }

    pwm_config config = pwm_get_default_config();
    //Change default config to include divider
    pwm_config_set_clkdiv(&config, (float)clk_div);

    //Set wrap number (20s)
    pwm_config_set_wrap(&config, period);

    //Initialize the slice found earlier
    pwm_init(slice_num, &config, false);
    // Enable PWM after setup is complete
    pwm_set_enabled(slice_num, true);
}

void button_setup(uint lift_button_pin, uint lower_button_pin){
    //Prep Button GPIOs
    gpio_init(lift_button_pin);
    gpio_init(lower_button_pin);

    //Directionally pressing down makes the action
    gpio_set_dir(lift_button_pin, GPIO_IN);
    gpio_set_dir(lower_button_pin, GPIO_IN);

    gpio_pull_up(lift_button_pin);
    gpio_pull_up(lower_button_pin);

    //Set up interrupt (event is falling edge on button press)
    // 0x4 is edge low from SDK docs
    gpio_set_irq_enabled_with_callback(lift_button_pin, 0x04, 1, check_button_callback);
    gpio_set_irq_enabled_with_callback(lower_button_pin, 0x04, 1, check_button_callback);
}

//UF2 File, Init to defaults
int main(){

    stdio_init_all();

    //Maybe struct with info on button + speed combined?
    button_setup(LIFT_UP_GPIO, LOWER_DOWN_GPIO);
    pwm_pin_setup(PWM_CONTROL_GPIO);

    //Potential to adjust speeds, maybe ask user later?
    uint lift_speed = 180;
    uint lower_speed = 0; 

    // while(true){
    //     uint degree = check_button(LIFT_UP_GPIO, LOWER_DOWN_GPIO, lift_speed, lower_speed); 
    //     //Current servo accepts PWM between 500-2500mus for max values)
    //     int duty = (((float)(ROTATE_MAX - ROTATE_MIN) / 180) * degree) + ROTATE_MIN;
    //     pwm_set_gpio_level(0, duty);
    // }
    while (true){
        tight_loop_contents();
    }
}




