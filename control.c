#include "control.h"
#include "timers.h"
#include "messages.h"
#include "ports.h"
#include "adc.h"

#include <avr/io.h>
#include <stdlib.h>

// Состояния системы управления
#define CONTROL_STATE_IDLE	        0   // Простой системы управления
#define CONTROL_STATE_PWM           1   // Управление ШИМ
#define CONTROL_STATE_PWM_DELAY     2   // Задержка после изменения частоты ШИМ
#define CONTROL_STATE_ADC           3   // Опрос АЦП
#define CONTROL_STATE_ADC_REPORT    4
#define CONTROL_STATE_REPORT        5   // Передача состояния объектов управления
#define CONTROL_STATE_EXECUTE       6   // Исполнение принятой команды управления

// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Объявление приватных констант
#define CONTROL_PWM_MIN     0
#define CONTROL_PWM_MAX     255

// Объявление приватных функций
void send_report(void);

// Объявление приватных переменных
unsigned int control_pwm;

// Указатель для работы с параметром - массивом команды
unsigned char *cmd_ptr;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;


void control_init(void)
{
    ports_configure();
    
    control_pwm = CONTROL_PWM_MIN;
    
    control_state_prev  = CONTROL_STATE_IDLE;
    control_state = control_state_prev;
    
    start_gtimer(GTIMER_CONTROL_REPORT);
}

void control_proc(void)
{
    switch (control_state_prev)
    {
    case CONTROL_STATE_IDLE:
        if (get_message(MSG_CONTROL_CMD_RCV))
            control_state = CONTROL_STATE_EXECUTE;
        else if (get_gtimer(GTIMER_CONTROL_REPORT) >= REP_TIMEOUT)
            control_state = CONTROL_STATE_PWM;
        break;
    case CONTROL_STATE_PWM:
        if (get_message(MSG_PWM_SET_OK))
        {
            if (control_pwm < CONTROL_PWM_MAX) control_pwm++;
            else control_pwm = CONTROL_PWM_MIN;
            
            control_state = CONTROL_STATE_PWM_DELAY;
        }
        else if (get_message(MSG_PWM_SET_ERR))
        {
            control_pwm = CONTROL_PWM_MIN;
            
            control_state = CONTROL_STATE_PWM_DELAY;
        }
        break;
    case CONTROL_STATE_PWM_DELAY:
        if (get_gtimer(GTIMER_CONTROL_PWM_ADC) >= PWM_TIMEOUT)
            control_state = CONTROL_STATE_ADC;
        break;
    case CONTROL_STATE_ADC:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_state = CONTROL_STATE_ADC_REPORT;
        }            
        break;
    case CONTROL_STATE_ADC_REPORT:
        control_state = CONTROL_STATE_PWM;
        break;
    case CONTROL_STATE_REPORT:
        control_state = CONTROL_STATE_IDLE;
        break;
    case CONTROL_STATE_EXECUTE:
        control_state = CONTROL_STATE_REPORT;
        break;
    default: break;
    }
    
    if (control_state != control_state_prev)
    {
        switch (control_state)
        {
        case CONTROL_STATE_IDLE:
            stop_gtimer(GTIMER_CONTROL_REPORT);
            start_gtimer(GTIMER_CONTROL_REPORT);
            break;
        case CONTROL_STATE_PWM:
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_PWM_DELAY:
            stop_gtimer(GTIMER_CONTROL_PWM_ADC);
            start_gtimer(GTIMER_CONTROL_PWM_ADC);
            break;
        case CONTROL_STATE_ADC:
            send_message(MSG_ADC_GET_0);
            break;
        case CONTROL_STATE_ADC_REPORT:
            send_report();
            break;
        case CONTROL_STATE_REPORT:
            send_report();
            break;
        case CONTROL_STATE_EXECUTE:        
            break;
        default: break;
        }
    }
    
    control_state_prev = control_state;
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
