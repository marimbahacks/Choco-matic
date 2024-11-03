#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define ROTATE_MAX 2500
#define ROTATE_MIN 500

#define PULL_UP_GPIO 2
#define DROP_DOWN_GPIO 16


int check_button(int pull_up_status, int drop_down_status){
    if (!gpio_get(pull_up_status)){
        return 180;
    }
    else if (!gpio_get(drop_down_status)){
        return 0;
    }
    else {
        return 90;
    }
}
//UF2 File, Init to defaults
int main(){

    stdio_init_all();

    //State GPIO 0 is allocated to PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    pwm_set_gpio_level(0, 0);

    //Prep button GPIOs
    gpio_init(PULL_UP_GPIO);
    gpio_init(DROP_DOWN_GPIO);

    //Directionally pressing down makes the action
    gpio_set_dir(PULL_UP_GPIO, GPIO_IN);
    gpio_set_dir(DROP_DOWN_GPIO, GPIO_IN);

    gpio_pull_up(PULL_UP_GPIO);
    gpio_pull_up(DROP_DOWN_GPIO);



    //Find PWM slice
    uint slice_num = pwm_gpio_to_slice_num(0);
    uint channel = pwm_gpio_to_channel(0);

    //50hz stuff?
    uint32_t clock = clock_get_hz(clk_sys);
    uint32_t div = clock / (20000 * 50);

    if (div < 1){
        div = 1;
    }
    if (div > 255){
        div = 255;
    }

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, (float)div);


    //Set wrap number
    pwm_config_set_wrap(&config, 20000);
    //3 is 25% dudy cycle for some reason?
    //pwm_set_chan_level(slice_num, PWM_CHAN_A, 5000);

    pwm_init(slice_num, &config, false);
    //default clock is 125 MHz, divider 
    //initalized to 1, counter is stepped every 8ns
    //wrap = freq clock/ f-1 or f = freq q / (wrap +1)
    pwm_set_enabled(slice_num, true);

    
    while(1){
        int degree = check_button(PULL_UP_GPIO, DROP_DOWN_GPIO); 
        int duty = (((float)(ROTATE_MAX - ROTATE_MIN) / 180) * degree) + ROTATE_MIN;
        pwm_set_gpio_level(0, duty);
    }
   

}




