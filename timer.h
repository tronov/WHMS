#ifndef TIMER_H_
#define TIMER_H_

#include <avr/wdt.h>

void timer_init(void);

unsigned char get_stimer(void);

#endif /* TIMER0_H_ */