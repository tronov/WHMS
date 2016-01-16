#include "pwm.h"

#include "timers.h"

#include <avr/io.h>

// Состояния ШИМ
#define PWM_STATE_FIX   0   // Поддержание постоянной частоты ШИМ
#define PWM_STATE_GROW  1   // Повышение частоты ШИМ
#define PWM_STATE_MAX   2   // Установление максимальной частоты ШИМ
#define PWM_STATE_FALL  3   // Понижение частоты ШИМ
#define PWM_STATE_MIN   4   // Установление миниимальной частоты ШИМ

// Новое и предыдущее состояния системы управления
unsigned char pwm_state, pwm_state_prev;

void pwm_init()
{
    
    // WGM13:WGM10
    // 0101 Fast PWM 8 bit
    // 0110 Fast PWM 9 bit
    // 0111 Fast PWM 10 bit
    
    TCCR1A = (3 << COM1A0)  // Прямой ШИМ на канале 1A
           | (3 << COM1B0)  // Прямой ШИМ на канале 1B
           | (0 << WGM11)   //
           | (1 << WGM10);  //
           
    TCCR1B = (1 << CS11);   // Частота счетчика CLK/8
           
    //TCCR1B = (0 << WGM13)   // 
           //| (1 << WGM12)   // 
           //| (1 << CS11);   // Частота счетчика CLK/8
    
    OCR1A = 254;
    //OCR1B = 128;
    
    pwm_state_prev = PWM_STATE_MIN;
    pwm_state = pwm_state_prev;
    
    start_gtimer(GTIMER_PWM);
}

void pwm_proc()
{
    switch (pwm_state_prev)
    {
    case PWM_STATE_FIX:
        break;
    case PWM_STATE_GROW:
        break;
    case PWM_STATE_MAX:
        break;
    case PWM_STATE_FALL:
        break;
    case PWM_STATE_MIN:
        break;
    }
    
    if(pwm_state != pwm_state_prev)
    {
        switch (pwm_state)
        {
        case PWM_STATE_FIX:
            break;
        case PWM_STATE_GROW:
            break;
        case PWM_STATE_MAX:
            break;
        case PWM_STATE_FALL:
            break;
        case PWM_STATE_MIN:
            break;
        }
    }
    
    pwm_state_prev = pwm_state;
}