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

    MotorControl motor;
    //Default layout
    motor.button_setup(); 
    motor.pwm_pin_setup(); 
    motor.controller_enable_pin_setup();


    //Potential to adjust speeds, maybe ask user later?
    uint lift_speed = 180;
    uint lower_speed = 0; 
    uint32_t time = to_ms_since_boot(get_absolute_time());



    while (true){
       // tight_loop_contents();
        motor.check_button();
    
    }
}




