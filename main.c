#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define ROTATE_MAX 2500
#define ROTATE_MIN 500

#define PWM_CONTROL_GPIO 0
#define PULL_UP_GPIO 2
#define DROP_DOWN_GPIO 16


int check_button(int pull_up_status, int drop_down_status){
    //If green button IS pressed down, move at full speed clockwise (up)
    if (!gpio_get(pull_up_status)){
        return 180;
    }
    //If red button is pressed down & green is not pressed, move full speed counter-clockwise (down)
    else if (!gpio_get(drop_down_status)){
        return 0;
    }
    //In all other scenarios, neutral motor speed/position
    return 90;
    
}
//UF2 File, Init to defaults
int main(){

    stdio_init_all();

    //State GPIO 0 is allocated to PWM
    gpio_set_function(PWM_CONTROL_GPIO, GPIO_FUNC_PWM);
    pwm_set_gpio_level(PWM_CONTROL_GPIO, 0);

    //Prep button GPIOs
    gpio_init(PULL_UP_GPIO);
    gpio_init(DROP_DOWN_GPIO);

    //Directionally pressing down makes the action
    gpio_set_dir(PULL_UP_GPIO, GPIO_IN);
    gpio_set_dir(DROP_DOWN_GPIO, GPIO_IN);

    gpio_pull_up(PULL_UP_GPIO);
    gpio_pull_up(DROP_DOWN_GPIO);



    //Find hardware PWM information
    uint slice_num = pwm_gpio_to_slice_num(PWM_CONTROL_GPIO);
    uint channel = pwm_gpio_to_channel(PWM_CONTROL_GPIO);

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

    //Initialize the slices found earlier
    pwm_init(slice_num, &config, false);
    // Enable PWM after setup is complete
    pwm_set_enabled(slice_num, true);

    //period = clock / Hz (not MHz)
    while(true){
        int degree = check_button(PULL_UP_GPIO, DROP_DOWN_GPIO); 
        //Current servo accepts PWM between 500-2500mus for max values)
        int duty = (((float)(ROTATE_MAX - ROTATE_MIN) / 180) * degree) + ROTATE_MIN;
        pwm_set_gpio_level(0, duty);
    }
   

}




