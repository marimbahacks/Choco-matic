#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

#define ROTATE_MAX 2500
#define ROTATE_MIN 500

#define PWM_CONTROL_GPIO 0
#define PWM_REVERSE_CONTROL_GPIO 14
#define LIFT_UP_GPIO 2
#define LOWER_DOWN_GPIO 16
#define ENABLE_PIN_1 18
#define ENABLE_PIN_2 19

#define DEBOUNCE_TIMER 50

volatile uint interrupt_flag = 0;
volatile enum gpio_irq_level interrupt_state = GPIO_IRQ_LEVEL_LOW;

uint degree_conversion(uint degree){
    return (((float)(ROTATE_MAX - ROTATE_MIN) / 180) * degree) + ROTATE_MIN;
}

void set_rotation(int pin_pressed, enum gpio_irq_level button_state){

    
    //6250 period, let's do half at first?
    if (interrupt_state == GPIO_IRQ_EDGE_FALL){
        //90 should be neutral state
        pwm_set_gpio_level(pin_pressed, degree_conversion(90));
    }
    //If green button is pressed down, move at full speed clockwise (up)
    if (pin_pressed == LIFT_UP_GPIO){
        //Max speed forward
        pwm_set_gpio_level(pin_pressed, degree_conversion(180));
    }   
    //If red button is pressed down & green is not pressed, move full speed counter-clockwise (down)
    else if (pin_pressed == LOWER_DOWN_GPIO){
        //Max speed backward
        pwm_set_gpio_level(pin_pressed, degree_conversion(0));

    }
    //In all other scenarios, neutral motor speed/position
    printf("No control pin pressed \n");
    pwm_set_gpio_level(pin_pressed, degree_conversion(90));
    //I have a bone to pick with MISRA not allowing multiple return statements
}

//Move to interrupts
void check_button_callback(uint control_pin, uint32_t events){
   //printf("Interupt triggered on GPIO %d with event %d \n", control_pin, events);
   interrupt_flag = control_pin;
   interrupt_state = gpio_get(control_pin);
   printf("Interrupt state: %d \n", interrupt_state);   
}

void pwm_pin_setup(uint control_pin){
    //Set PWM functionality with default of 0
    gpio_set_function(control_pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(control_pin, 0);

    //Find hardware PWM information
    uint slice_num = pwm_gpio_to_slice_num(control_pin);
    uint channel = pwm_gpio_to_channel(control_pin);

    //PWM Adjustment Levers (12V DC Motor estimated start: 20kHz instead of 50Hz servo
    uint period = 6250;
    uint desired_output_hz = 10000;

    //Get base of system clock (should be 125MHz)
    uint32_t clock = clock_get_hz(clk_sys);
    //Clock divider needed as lowest PWM freq of pi pico is 1.9kHz without
    // div = clock (125MHz) / period of 6250 * freq desired (20000) makes clk div = 1
    uint32_t clk_div = clock / (period * desired_output_hz);
    printf("Clock divider is %d for PWM Pin %d\n", clk_div, control_pin);
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

    //Set wrap number
    pwm_config_set_wrap(&config, period);

    pwm_set_chan_level(slice_num, channel, 6249);

    //Initialize the slice found earlier
    pwm_init(slice_num, &config, false);
    // Enable PWM after setup is complete
    pwm_set_enabled(slice_num, true);
    printf("PWM Pin %d is enabled\n", control_pin);
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
    gpio_set_irq_enabled_with_callback(lift_button_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 1, check_button_callback);
    
    gpio_set_irq_enabled_with_callback(lower_button_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 1, check_button_callback);
}

bool debounce_check(uint32_t previous_time, uint pin){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - previous_time > DEBOUNCE_TIMER){
        return true;
    }
    return false;
}

//UF2 File, Init to defaults
int main(){

    stdio_init_all();

    //Maybe struct with info on button + speed combined?
    button_setup(LIFT_UP_GPIO, LOWER_DOWN_GPIO);
    pwm_pin_setup(PWM_CONTROL_GPIO);
    pwm_pin_setup(PWM_REVERSE_CONTROL_GPIO);

    gpio_init(ENABLE_PIN_1);
    gpio_init(ENABLE_PIN_2);

    gpio_set_dir(ENABLE_PIN_1, GPIO_OUT);
    gpio_set_dir(ENABLE_PIN_2, GPIO_OUT);

    gpio_put(ENABLE_PIN_1, 1);
    gpio_put(ENABLE_PIN_2, 1);
    //Potential to adjust speeds, maybe ask user later?
    uint lift_speed = 180;
    uint lower_speed = 0; 
    uint32_t time = to_ms_since_boot(get_absolute_time());

    while(true){
        printf("Starting Forward\n");
        pwm_set_gpio_level(PWM_CONTROL_GPIO, 6248);
        pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 0);
        sleep_ms((uint32_t)5000);
        printf("Stopping\n");
        pwm_set_gpio_level(PWM_CONTROL_GPIO, 0);
        pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 0);
        sleep_ms((uint32_t)5000);
        printf("Starting Reverse\n");
        pwm_set_gpio_level(PWM_CONTROL_GPIO, 0);
        pwm_set_gpio_level(PWM_REVERSE_CONTROL_GPIO, 6248);
        sleep_ms((uint32_t)5000);
    }

    // set_rotation(interrupt_flag, interrupt_state);


    // while (true){
    //     tight_loop_contents();

    //     if (interrupt_state == GPIO_IRQ_EDGE_FALL){
    //         printf("Pin %d is no longer pressed\n", interrupt_flag);
    //         set_rotation(interrupt_flag, interrupt_state);
    //         interrupt_flag = 0;
    //     }
    //     else if (interrupt_flag != 0 && debounce_check(time, interrupt_flag)){
    //         set_rotation(interrupt_flag, interrupt_state);
    //         printf("Interrupt from pin %d success\n", interrupt_flag);
    //         time = to_ms_since_boot(get_absolute_time());
    //         interrupt_flag = 0;
    //     }
    //     else if (interrupt_flag != 0){
    //         printf("Debounced from pin %d\n", interrupt_flag);
    //         interrupt_flag = 0;
    //     }
    
    // }
}




