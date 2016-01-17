#include "pwm.h"

#include "messages.h"

#include <avr/io.h>

#define PWM_MIN         0
#define PWM_MAX         255

// Состояния ШИМ
#define PWM_STATE_FIX   0   // Поддержание установленной частоты ШИМ
#define PWM_STATE_MIN   1   // Установление миниимальной частоты ШИМ
#define PWM_STATE_GROW  2   // Увеличение частоты ШИМ

// Новое и предыдущее состояния системы управления
unsigned char pwm_state, pwm_state_prev;

void pwm_init()
{
    TCCR1A = (3 << COM1A0)  // Прямой ШИМ на канале 1A
           | (3 << COM1B0)  // Прямой ШИМ на канале 1B
           | (0 << WGM11)   //
           | (1 << WGM10);  //
           
    TCCR1B = (1 << CS11);   // Частота счетчика CLK/8
    
    OCR1A = PWM_MIN;
    
    pwm_state_prev = PWM_STATE_MIN;
    pwm_state = pwm_state_prev;
}

void pwm_proc()
{
    switch (pwm_state_prev)
    {
    case PWM_STATE_FIX:
        if (get_message(MSG_PWM_GROW)) pwm_state = PWM_STATE_GROW;
        else if (get_message(MSG_PWM_SET_MIN)) pwm_state = PWM_STATE_MIN;
        break;
    case PWM_STATE_MIN:
    case PWM_STATE_GROW:
        pwm_state = PWM_STATE_FIX;
        break;
    default: break;
    }
    
    if(pwm_state != pwm_state_prev)
    {
        switch (pwm_state)
        {
        case PWM_STATE_FIX:
            break;
        case PWM_STATE_MIN:
            OCR1A = PWM_MIN;
            break;
        case PWM_STATE_GROW:
            if (OCR1A < PWM_MAX)
            {
                OCR1A++;
                send_message(MSG_PWM_GROW_OK);
            }
            else send_message(MSG_PWM_MAX);
            break;
        default: break;
        }
    }
    
    pwm_state_prev = pwm_state;
}