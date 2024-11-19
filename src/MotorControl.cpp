#include "MotorControl.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
/*
#define PWM_CONTROL_GPIO 0
#define PWM_REVERSE_CONTROL_GPIO 14
#define LIFT_UP_GPIO 2
#define LOWER_DOWN_GPIO 16
#define DEBOUNCE_TIMER 50
*/
volatile uint interrupt_flag = 0;
//False = low, true = high?
volatile uint32_t interrupt_state = GPIO_IRQ_EDGE_FALL;

//  GPIO_IRQ_LEVEL_LOW = 0x1u,  ///< IRQ when the GPIO pin is a logical 1
//     GPIO_IRQ_LEVEL_HIGH = 0x2u, ///< IRQ when the GPIO pin is a logical 0
//     GPIO_IRQ_EDGE_FALL = 0x4u,  ///< IRQ when the GPIO has transitioned from a logical 0 to a logical 1
//     GPIO_IRQ_EDGE_RISE = 0x8u, 

MotorControl::MotorControl()
   //To implement: percentage out of 6249
   //:m_desired_speed{desired_speed},
   :lift_up_gpio {2},
   lower_down_gpio {16},
   debounce_timer {50},
   pwm_control_gpio {0},
   pwm_reverse_control_gpio {14},
   time {to_ms_since_boot(get_absolute_time())}

{
}

void MotorControl::default_pwm_pin_setup(){
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


void MotorControl::default_button_setup(){
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

    //Set up interrupt (event is falling edge on button press)
    // 0x4 is edge low from SDK docs
    gpio_set_irq_enabled_with_callback(lift_button_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &check_button_callback);
    
    gpio_set_irq_enabled_with_callback(lower_button_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &check_button_callback);
}

void MotorControl::default_controller_enable_pin_setup(){
    controller_enable_pin_setup(18);
    controller_enable_pin_setup(19);
}

void MotorControl::controller_enable_pin_setup(uint enable){
    gpio_init(enable);
    gpio_set_dir(enable, GPIO_OUT);
    gpio_put(enable, 1);

}
void MotorControl::set_rotation(int pin_pressed, uint32_t button_state){
    if (interrupt_state == 4){
        //GPIO_IRQ_EDGE_FALL
        printf("Pin released/No pin is pressed\n");
        //Both PWM pins at 0 should be no movement
        pwm_set_gpio_level(pwm_control_gpio, 0);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 0);

    }
    //If green button is pressed down, move at full speed clockwise (up)
    //First PWM pin forward, 2nd backward
    else if (pin_pressed == lift_up_gpio){
        printf("Lift up pin is pressed\n");
        //Max speed forward
        pwm_set_gpio_level(pwm_control_gpio, 6249);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 0);

    }   
    //If red button is pressed down & green is not pressed, move full speed counter-clockwise (down)
    //First PWM pin backward, 2nd forward
    else if (pin_pressed == lower_down_gpio){
        printf("Lower down pin is pressed\n");
        //Max speed backward
        pwm_set_gpio_level(pwm_control_gpio, 0);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 6249);

    }
    else {
         //In all other scenarios, neutral motor speed/position
        printf("No control pin pressed \n");
        pwm_set_gpio_level(pwm_control_gpio, 0);
        pwm_set_gpio_level(pwm_reverse_control_gpio, 0);
    }
}

void static check_button_callback(uint control_pin, uint32_t event_mask){
    //printf("Interupt triggered on GPIO %d with event %d \n", control_pin, events);
    interrupt_flag = control_pin;
    printf("Event Mask: %d", event_mask);
    printf("Callback Pin: %d  triggered at state: %d\n", control_pin, interrupt_state);
    interrupt_state = event_mask;
    if (debounce_check(time, to_ms_since_boot(get_absolute_time()))){
        printf("Passed debounce check\n");
        set_rotation(control_pin, interrupt_state);
        time = to_ms_since_boot(get_absolute_time());
    }
    else{
       printf("Failed debounce check\n");
    }
   
   
}

bool MotorControl::debounce_check(uint32_t previous_time, uint pin){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - previous_time > debounce_timer){
        return true;
    }
    return false;
}

//Spencer: servo control holdover, not needed in DC motor
// uint MotorControl::degree_conversion(uint degree){
//     return (((float)(ROTATE_MAX - ROTATE_MIN) / 180) * degree) + ROTATE_MIN;
// }