#include "control.h"
#include "timers.h"
#include "messages.h"
#include "ports.h"
#include "adc.h"

#include <avr/io.h>
#include <stdlib.h>

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
#define CONTROL_STATE_RESET_PWM     10  // Сброс частоты ШИМ на минимальное значение
#define CONTROL_STATE_REPORT        11  // Анализ данных и отправка отчёта по UART


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




// Объявление приватных функций
void send_report(void);

// Объявление приватных переменных
unsigned int control_pwm;
volatile double control_g,
                control_u,
                control_i;

// Указатель для работы с параметром - массивом команды
unsigned char *cmd_ptr;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;


void control_init(void)
{
    ports_configure();
    
    control_pwm = CONTROL_PWM_MIN;
    
    control_state_prev  = CONTROL_STATE_R_ON;
    control_state = control_state_prev;
    
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
        if (get_gtimer(GTIMER_CONTROL_PWM))
            control_state = CONTROL_STATE_GET_U;
        break;
    case CONTROL_STATE_GET_U:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_u = *((double *) get_message_param(MSG_ADC_GET_OK));
            
            /* TODO:: внести в массив */
            
            control_state = CONTROL_STATE_GET_I;
        }
        break;
    case CONTROL_STATE_GET_I:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_i = *((double *) get_message_param(MSG_ADC_GET_OK));
            
            /* TODO:: внести в массив */
            
            if (control_pwm < CONTROL_PWM_MAX)
                control_state = CONTROL_STATE_SET_PWM;
            else
                control_state = CONTROL_STATE_RESET_PWM;
        }        
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
        case CONTROL_STATE_RESET_PWM:
            control_pwm = CONTROL_PWM_MIN;
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_REPORT:
            //send_report();
            break;
        default: break;
        }
        
        control_state_prev = control_state;
    }
}


void send_report()
{
    char buffer [10];
    unsigned char buffer_i;
    
    report_i = 0;
    
    dtostrf( *((double *)get_message_param(MSG_ADC_GET_OK)), 1, 5, (char *) &buffer);
        
    for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
    {
        report[report_i++] = buffer[buffer_i++];
    }

    report[report_i++] = REP_LF;
    report[report_i++] = REP_CR;
    report[report_i++] = REP_EOL;
    
    send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
}
