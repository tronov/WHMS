#pragma once

// Settings
#define USE_GLOBAL_TIMERS

#define MS      10L         // Интервал системного таймера ~0.1 мс
#define SEC     1000 * MS
#define MIN     60   * SEC
#define HOUR    60   * MIN


#define TIMER_UART      0
#define TIMER_CONTROL   1
#define TIMERS_NUMBER   2


#ifdef USE_GLOBAL_TIMERS
    #define GTIMER_ADC              0
    #define GTIMER_CONTROL          1
    #define GTIMER_CONTROL_R_ON     2
    #define GTIMER_CONTROL_R_OFF    3
    #define GTIMER_CONTROL_PWM      4
    #define GTIMER_UART             5
    #define GTIMERS_NUMBER 	        6
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
