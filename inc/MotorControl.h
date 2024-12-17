#ifndef _MOTOR_CONTROL_H
#define _MOTOR_CONTROL_H

#include <stdio.h>
#include "pico/stdlib.h"

class MotorControl{
    public:
        MotorControl();
        //Remove default methods and overload
        void default_pwm_pin_setup();   //spencer: remove
        void pwm_pin_setup(uint control_pin);
        void default_button_setup();    //spencer: remove
        void button_setup(uint lift_button_pin, uint lower_button_pin);
        void default_controller_enable_pin_setup(); //spencer : remove
        void controller_enable_pin_setup(uint enable);
        void set_rotation(int pin_pressed);
        void check_button();
    
        private:
        //int m_desired_speed;
        //Dont have close to 255 GPIO pin options, save some memory
        uint8_t lift_up_gpio;
        uint8_t lower_down_gpio;
        uint debounce_timer;
        uint8_t pwm_control_gpio;
        uint8_t pwm_reverse_control_gpio;
        uint32_t time;

        //static void dummy_handler(uint control_pin, uint32_t event_mask);
        bool debounce_check(uint32_t previous_time, uint pin);
        //NOT SAFE IF MORE THAN 1 OBJECT EXISTS - need to make singleton
        //void member_helper(uint control_pin, uint32_t event_mask, MotorControl* motor);
        //Spencer: remember previous PWM pin setup
        // uint prev_slice_num;
        // uint prev_channel;
        // uint prev_period;
        // uint prev_desired_output_hz;
        // uint32_t prev_clock;
        // uint32_t prev_clk_div;
    
};






#endif