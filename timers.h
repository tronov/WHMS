#ifndef TIMERS_H_
#define TIMERS_H_

// Settings
#define USE_GLOBAL_TIMERS

#define SEC     100    // Интервал системного таймера ~10 мс
#define MIN     60 * SEC
#define HOUR    60 * MIN


#define TIMER_UART      0
#define TIMER_CONTROL   1
#define TIMERS_NUMBER   2


#ifdef USE_GLOBAL_TIMERS
    #define GTIMER_ADC      0
    #define GTIMER_UART     1
    #define GTIMER_CONTROL  2
    #define GTIMERS_NUMBER 	3
#endif

void timers_init(void);

void timers_proc(void);

unsigned int get_timer(unsigned char timer);

void reset_timer(unsigned char timer);

#ifdef USE_GLOBAL_TIMERS
    unsigned int get_gtimer(unsigned char gtimer);
    void stop_gtimer(unsigned char gtimer);
    void start_gtimer(unsigned char gtimer);
    void pause_gtimer(unsigned char gtimer);
    void resume_gtimer(unsigned char gtimer);
#endif

#endif /* TIMERS_H_ */