#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "MotorControl.h"

//Servo rotation parameters from motor specs
//#define ROTATE_MAX 2500
//#define ROTATE_MIN 500

// MC specific pins required to be turned on
// #define ENABLE_PIN_1 18
// #define ENABLE_PIN_2 19




//UF2 File, Init to defaults
int main(){

    stdio_init_all();

    //Maybe struct with info on button + speed combined?

    MotorControl motor;
    
    motor.default_button_setup(); //Default
    motor.default_pwm_pin_setup(); //Default
    motor.default_controller_enable_pin_setup();


    //Potential to adjust speeds, maybe ask user later?
    uint lift_speed = 180;
    uint lower_speed = 0; 
    uint32_t time = to_ms_since_boot(get_absolute_time());

    // while(true){
    //     printf("Starting Forward\n");
    //     pwm_set_gpio_level(PWM_CONTROL_GPIO, 6248);
    //     pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 0);
    //     sleep_ms((uint32_t)5000);
    //     printf("Stopping\n");
    //     pwm_set_gpio_level(PWM_CONTROL_GPIO, 0);
    //     pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 0);
    //     sleep_ms((uint32_t)5000);
    //     printf("                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          Reverse\n");
    //     pwm_set_gpio_level(PWM_CONTROL_GPIO, 0);
    //     pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 6248);
    //     sleep_ms((uint32_t)5000);
    // }

    // set_rotation(interrupt_flag, interrupt_state);


    while (true){
       // tight_loop_contents();
        motor.check_button();
        /*
        if (!gpio.get(motor.lift_up_gpio)){
            printf("Pin %d is no longer pressed\n", interrupt_flag);
            set_rotation(interrupt_flag, interrupt_state);
            interrupt_flag = 0;
        }
        else if (interrupt_flag != 0 && debounce_check(time, interrupt_flag)){
            set_rotation(interrupt_flag, interrupt_state);
            printf("Interrupt from pin %d success\n", interrupt_flag);
            time = to_ms_since_boot(get_absolute_time());
            interrupt_flag = 0;
        }
        else if (interrupt_flag != 0){
            printf("Debounced from pin %d\n", interrupt_flag);
            interrupt_flag = 0;
        }
        */
    
    }
}




