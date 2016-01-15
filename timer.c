#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define TIMER_PERIOD    100 // 0.1 ms
#define TIMER_PRELOAD   256 - TIMER_PERIOD

unsigned char stimer;

void timer_init()
{
    cli();
    TCCR0  = (1 << CS01);
    TCNT0  = TIMER_PRELOAD;
    TIMSK |= (1 << TOIE0);
    
    wdt_reset();
    
    WDTCR |= (1 << WDTOE) | (1 << WDE); // Включить сторожевой таймер
    
    WDTCR = (1 << WDE) | (1 << WDP2) | (1 << WDP0); // 512K циклов (~0,5 с)
    
    stimer = 0;
    sei();
}

ISR(TIMER0_OVF_vect)
{
    stimer++;
    TCNT0 = TIMER_PRELOAD;
}

unsigned char get_stimer()
{
    unsigned char result = stimer;
    stimer = 0;
    return result;
}