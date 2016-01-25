#include "pwm.h"

#include "messages.h"

#include <avr/io.h>

#define PWM_MIN         0
#define PWM_MAX         255

// Состояния ШИМ
#define PWM_STATE_FIX   0   // Поддержание установленной частоты ШИМ
#define PWM_STATE_SET   1   // Установление частоты ШИМ

// Новое и предыдущее состояния системы управления
unsigned char pwm_state, pwm_state_prev;

void pwm_init()
{
    TCCR1A = (3 << COM1A0)  // Обратный ШИМ на канале 1A
           | (3 << COM1B0)  // Обратный ШИМ на канале 1B
           | (0 << WGM11)   //
           | (1 << WGM10);  //
           
    TCCR1B = (1 << CS11);   // Частота счетчика CLK/8
    
    OCR1A = PWM_MIN;
    
    pwm_state_prev = PWM_STATE_FIX;
    pwm_state = pwm_state_prev;
}

void pwm_proc()
{
    switch (pwm_state_prev)
    {
    case PWM_STATE_FIX:
        if (get_message(MSG_PWM_SET)) pwm_state = PWM_STATE_SET;
        break;
    case PWM_STATE_SET:
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
        case PWM_STATE_SET:
            {
                unsigned int param = *((unsigned int *) get_message_param(MSG_PWM_SET));
                if (param <= PWM_MAX && param >= PWM_MIN)
                {
                    OCR1A = param;
                    send_message(MSG_PWM_SET_OK);
                }
                else send_message(MSG_PWM_SET_ERR);
            }
            break;
        default: break;
        }
        
        pwm_state_prev = pwm_state;
    }
}