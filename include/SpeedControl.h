#ifndef _SPEED_CONTROL_H
#define _SPEED_CONTROL_H

#include <stdio.h>
#include "pico/stdlib.h"

class SpeedControl{
    private:
        int m_desired_speed;
    public:
        SpeedControl(int desired_speed);
        void set_rotation(int pin_pressed, enum gpio_irq_level button_state);

    
};






#endif