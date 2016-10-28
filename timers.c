#include "timers.h"

#include "timer.h"

volatile unsigned int timers_buffer[TIMERS_NUMBER];

#ifdef USE_GLOBAL_TIMERS
    #define GTIMER_STOPPED  0
    #define GTIMER_RUNNING  1
    #define GTIMER_PAUSED   2

    volatile unsigned int gtimers_buffer[GTIMERS_NUMBER];
    volatile unsigned char gtimers_states_buffer[GTIMERS_NUMBER];
#endif

void timers_init()
{
    unsigned char i;
    for(i = 0; i < TIMERS_NUMBER; i++)
        timers_buffer[i] = 0;

#ifdef USE_GLOBAL_TIMERS
    for(i = 0; i < GTIMERS_NUMBER; i++)
    {
        gtimers_buffer[i] = 0;
        gtimers_states_buffer[i] = GTIMER_STOPPED;
    }        
#endif
    
    // Инициализация аппаратного таймера
    timer_init();
}

void timers_proc()
{
    // Сброс сторожевого таймера
    wdt_reset();
    
	unsigned char i = 0;
	unsigned int x = get_stimer();

	if (x > 0)
    {
    	for (i = 0; i < TIMERS_NUMBER; i++)
        {
      	    timers_buffer[i] += x;
    	}
#ifdef USE_GLOBAL_TIMERS
        for (i = 0; i < GTIMERS_NUMBER; i++)
        {
            if (gtimers_states_buffer[i] == GTIMER_RUNNING)
                gtimers_buffer[i] += x;
        }
#endif
	}
}

unsigned int get_timer(unsigned char timer)
{
    return timers_buffer[timer];
}

void reset_timer(unsigned char timer)
{
    timers_buffer[timer] = 0;
}

#ifdef USE_GLOBAL_TIMERS
unsigned int get_gtimer(unsigned char gtimer)
{
    return gtimers_buffer[gtimer];
}

void stop_gtimer(unsigned char gtimer)
{
    gtimers_states_buffer[gtimer] = GTIMER_STOPPED;
}

void start_gtimer(unsigned char gtimer)
{
    if(gtimers_states_buffer[gtimer] == GTIMER_STOPPED)
    {
        gtimers_buffer[gtimer] = 0;
        gtimers_states_buffer[gtimer] = GTIMER_RUNNING;
    }
}

void pause_gtimer(unsigned char gtimer)
{
    if(gtimers_states_buffer[gtimer] == GTIMER_RUNNING)
        gtimers_states_buffer[gtimer] = GTIMER_PAUSED;
}

void resume_gtimer(unsigned char gtimer)
{
    if(gtimers_states_buffer[gtimer] == GTIMER_PAUSED)
        gtimers_states_buffer[gtimer] = GTIMER_RUNNING;
}
#endif