#include "uart.h"
#include "control.h"

#include "timers.h"
#include "messages.h"

#include <avr/io.h>

int main(void)
{
    timers_init();
    messages_init();

    control_init();
    uart_init();
    
    while(1)
    {
        timers_proc();

        uart_proc();
        control_proc();
        
        messages_proc();
    }
    return 0;
}