#include "control.h"
#include "timers.h"
#include "messages.h"
#include "ports.h"
#include "adc.h"
#include "calc.h"



// Состояния системы управления
#define CONTROL_STATE_IDLE	        0   // Простой системы управления
#define CONTROL_STATE_R_ON          1   // Включение реле
#define CONTROL_STATE_R_ON_TIMEOUT  2   // Тайм-аут переходных процессов после включения реле
#define CONTROL_STATE_GET_G         3   // Измерение проводимости (Gin)
#define CONTROL_STATE_R_OFF         4   // Выключение реле
#define CONTROL_STATE_R_OFF_TIMEOUT 5   // Тайм-аут переходных процессов после выключения реле
#define CONTROL_STATE_SET_PWM       6   // Установка частоты ШИМ
#define CONTROL_STATE_PWM_TIMEOUT   7   // Тайм-аут переходных процессов после изменения частоты ШИМ
#define CONTROL_STATE_GET_U         8   // Измерение потенциала катода полярографической ячейки (Uin)
#define CONTROL_STATE_GET_I         9   // Измерение тока полярографической ячейки (Iin)
#define CONTROL_STATE_CALCULATE     10  // Анализ считанных данных
#define CONTROL_STATE_RESET_PWM     11  // Сброс частоты ШИМ на минимальное значение
#define CONTROL_STATE_REPORT        12  // Отправка отчёта по UART


// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Объявление приватных констант
#define CONTROL_PWM_MIN         0
#define CONTROL_PWM_MAX         255

// Timeout in ticks (1 tick ~ 0.1 millisecond)
#define CONTROL_TIMEOUT         (1 * SEC)   // Тайм-аут запуска измерений
#define CONTROL_TIMEOUT_R_ON    (100 * MS)  // Тайм-аут переходных процессов после подачи напряжения на реле
#define CONTROL_TIMEOUT_R_OFF   (100 * MS)  // Тайм-аут переходных процессов после измерения напряжения Gin
#define CONTROL_TIMEOUT_PWM     (5 * MS)    // Тайм-аут переходных процессов после изменения частоты ШИМ

// Объявление приватных переменных
unsigned int control_pwm;
volatile double control_g;
volatile double control_u;
volatile double control_i;
volatile unsigned char rushes_found = 0;

void control_init(void)
{
    ports_configure();
    
    control_pwm = CONTROL_PWM_MIN;
    
    control_state_prev  = CONTROL_STATE_R_ON;
    control_state = control_state_prev;
    
    reset_calculations();
    
    start_gtimer(GTIMER_CONTROL);
}

void control_proc(void)
{
    switch (control_state_prev)
    {
    case CONTROL_STATE_IDLE:
        if (get_gtimer(GTIMER_CONTROL) >= CONTROL_TIMEOUT)
            control_state = CONTROL_STATE_R_ON;
        break;
    case CONTROL_STATE_R_ON:
        control_state = CONTROL_STATE_R_ON_TIMEOUT;
        break;
    case CONTROL_STATE_R_ON_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_R_ON) > CONTROL_TIMEOUT_R_ON)
            control_state = CONTROL_STATE_GET_G;
        break;
    case CONTROL_STATE_GET_G:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_g = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_R_OFF;
        }
        break;
    case CONTROL_STATE_R_OFF:
        control_state = CONTROL_STATE_R_OFF_TIMEOUT;
        break;
    case CONTROL_STATE_R_OFF_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_R_OFF) > CONTROL_TIMEOUT_R_OFF)
            control_state = CONTROL_STATE_SET_PWM;
        break;
    case CONTROL_STATE_SET_PWM:
        if (get_message(MSG_PWM_SET_OK))
            control_state = CONTROL_STATE_PWM_TIMEOUT;
        else if (get_message(MSG_PWM_SET_ERR))
            control_state = CONTROL_STATE_RESET_PWM;
        break;
    case CONTROL_STATE_PWM_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_PWM) > CONTROL_TIMEOUT_PWM)
            control_state = CONTROL_STATE_GET_U;
        break;
    case CONTROL_STATE_GET_U:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_u = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_GET_I;
        }
        break;
    case CONTROL_STATE_GET_I:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_i = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_CALCULATE;
        }
        break;
    case CONTROL_STATE_CALCULATE:
        if (rushes_found >= CONTROL_RUSHES_MAX)
            control_state = CONTROL_STATE_RESET_PWM;
        else if (control_pwm < CONTROL_PWM_MAX)
            control_state = CONTROL_STATE_SET_PWM;
        else
            control_state = CONTROL_STATE_RESET_PWM;
        break;
    case CONTROL_STATE_RESET_PWM:
        if (get_message(MSG_PWM_SET_OK) || get_message(MSG_PWM_SET_ERR))
            control_state = CONTROL_STATE_REPORT;
        break;
    case CONTROL_STATE_REPORT:
        control_state = CONTROL_STATE_IDLE;
        break;
    default: break;
    }
    
    if (control_state != control_state_prev)
    {
        switch (control_state)
        {
        case CONTROL_STATE_IDLE:
            stop_gtimer(GTIMER_CONTROL);
            start_gtimer(GTIMER_CONTROL);
            break;
        case CONTROL_STATE_R_ON:
            Rel_PORT |= (1 << Rel_PORT_N);
            break;
        case CONTROL_STATE_R_ON_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_R_ON);
            start_gtimer(GTIMER_CONTROL_R_ON);
            break;
        case CONTROL_STATE_GET_G:
            send_message(MSG_ADC_GET_2);
            break;
        case CONTROL_STATE_R_OFF:
            Rel_PORT &= ~(1 << Rel_PORT_N);
            break;
        case CONTROL_STATE_R_OFF_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_R_OFF);
            start_gtimer(GTIMER_CONTROL_R_OFF);
            break;
        case CONTROL_STATE_SET_PWM:
            control_pwm++;
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_PWM_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_PWM);
            start_gtimer(GTIMER_CONTROL_PWM);
            break;
        case CONTROL_STATE_GET_U:
            send_message(MSG_ADC_GET_0);
            break;
        case CONTROL_STATE_GET_I:
            send_message(MSG_ADC_GET_1);
            break;
        case CONTROL_STATE_CALCULATE:
            if (rushes_found < CONTROL_RUSHES_MAX)
                rushes_found = make_calculations(control_u, control_i);
            break;
        case CONTROL_STATE_RESET_PWM:
            control_pwm = CONTROL_PWM_MIN;
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_REPORT:
            send_report(control_g);
            reset_calculations();
            rushes_found = 0;
            break;
        default: break;
        }
        
        control_state_prev = control_state;
    }
}
