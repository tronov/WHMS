#include "adc.h"
#include "pwm.h"
#include "uart.h"
#include "control.h"

#include "timers.h"
#include "messages.h"

#include <avr/io.h>

int main(void)
{
    timers_init();
    messages_init();

    adc_init();
    pwm_init();
    uart_init();
    control_init();
    
    while(1)
    {
        timers_proc();

        adc_proc();
        pwm_proc();
        uart_proc();
        control_proc();
        
        messages_proc();
    }
    return 0;
}