#include "ports.h"

void ports_configure()
{
    // Disable pull-up resistors
    MCUCR    |=  (1 << PUD);
    
    // Configure input ports
    Uin_DDR  &= ~(1 << Uin_PIN_N);
    Iin_DDR  &= ~(1 << Iin_PIN_N);
    Gin_DDR  &= ~(1 << Gin_PIN_N);

    // Configure output ports
    Uout_DDR |=  1 << Uout_PORT_N;
    Rel_DDR  |=  1 << Rel_PORT_N;
}