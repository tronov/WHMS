#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define TIMER_PRELOAD      0xb2     // 0xff - 0xb2 = 0x4e = 78 ~ 10 ms

unsigned char stimer;

void timer_init()
{
    cli();
    TCCR0  = (1<<CS02)|(0<<CS01)|(1<<CS00); // Установка предделителя
    TCNT0  = TIMER_PRELOAD;
    TIMSK |= (1<<TOIE0);
    
    wdt_reset();
    
    WDTCR |= (1<<WDTOE) | (1<<WDE); // Включить сторожевой таймер
    
    WDTCR = (1<<WDE)
           | (1<<WDP2) | (1<<WDP0); // Тайм-аут 512K циклов (~0,5 с)
    
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