#include "signals.h"

#include "ports.h"

unsigned char input_signals_buffer[INPUT_SIGNALS_NUMBER];

void signals_init()
{
    ports_configure();
    
    clr_signal(Uout);
    clr_signal(Rel);
    
    volatile unsigned char i;
    for(i = 0; i < INPUT_SIGNALS_NUMBER; i++) input_signals_buffer[i] = 0;
}

void signals_proc()
{
    input_signals_buffer[Uin] = !(Uin_PIN & (1<<Uin_PIN_N));
    input_signals_buffer[Iin] = !(Iin_PIN & (1<<Iin_PIN_N));
    input_signals_buffer[Gin] = !(Gin_PIN & (1<<Gin_PIN_N));

}

unsigned char get_signal(input_signal signal)
{
    if(signal >= INPUT_SIGNALS_NUMBER) return 0;
    
    return input_signals_buffer[signal];
}

void set_signal(output_signal signal)
{
    switch (signal)
    {
        case Uout:
            Uout_PORT &= ~(1<<Uout_PORT_N);
            break;
        case Rel:
            Rel_PORT  &= ~(1<<Rel_PORT_N);
            break;
        default: break;
    }
}

void clr_signal(output_signal signal)
{
    switch (signal)
    {
        case Uout:
		    Uout_PORT |= (1<<Uout_PORT_N);
            break;
        case Rel:
		    Rel_PORT  |= (1<<Rel_PORT_N);
            break;
        default: break;
    }
}