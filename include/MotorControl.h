#ifndef _MOTOR_CONTROL_H
#define _MOTOR_CONTROL_H

#include <stdio.h>
#include "pico/stdlib.h"

class MotorControl{
    private:
        int m_desired_speed;
        //Spencer: remember previous PWM pin setup
        // uint prev_slice_num;
        // uint prev_channel;
        // uint prev_period;
        // uint prev_desired_output_hz;
        // uint32_t prev_clock;
        // uint32_t prev_clk_div;

    public:
        MotorControl(int desired_speed);
        void set_rotation(int pin_pressed, enum gpio_irq_level button_state);
        void pwm_pin_setup(uint control_pin);
    
};






#endif