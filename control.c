#include "control.h"
#include "timers.h"
#include "messages.h"
#include "ports.h"
#include "adc.h"

#include <avr/io.h>
#include <stdlib.h>

// Состояния системы управления
#define CONTROL_STATE_IDLE	    0   // Простой системы управления
#define CONTROL_STATE_PWM       1   // Управление ШИМ
#define CONTROL_STATE_PWM_DELAY 2   // Задержка после изменения частоты ШИМ
#define CONTROL_STATE_ADC       3   // Опрос АЦП
#define CONTROL_STATE_REPORT    4   // Передача состояния объектов управления
#define CONTROL_STATE_EXECUTE   5   // Исполнение принятой команды управления

// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Объявление приватных функций
void send_report(void);

// Указатель для работы с параметром - массивом команды
unsigned char *cmd_ptr;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;


void control_init(void)
{
    ports_configure();
    
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
        if (get_message(MSG_PWM_GROW_OK))
            control_state = CONTROL_STATE_PWM_DELAY;
        else if (get_message(MSG_PWM_MAX))
        {
            send_message(MSG_PWM_SET_MIN);
            control_state = CONTROL_STATE_REPORT;
        }            
        break;
    case CONTROL_STATE_PWM_DELAY:
        if (get_gtimer(GTIMER_CONTROL_PWM_ADC) >= PWM_TIMEOUT)
            control_state = CONTROL_STATE_ADC;
        break;
    case CONTROL_STATE_ADC:
        if (get_message(MSG_ADC_COMPLETE))
        {
            control_state = CONTROL_STATE_PWM;
        }            
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
            send_message(MSG_PWM_GROW);
            break;
        case CONTROL_STATE_PWM_DELAY:
            stop_gtimer(GTIMER_CONTROL_PWM_ADC);
            start_gtimer(GTIMER_CONTROL_PWM_ADC);
            break;
        case CONTROL_STATE_ADC:
            send_message(MSG_ADC_READ_0);
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
    
    dtostrf(get_adc(), 1, 5, (char *) &buffer);
        
    for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
    {
        report[report_i++] = buffer[buffer_i++];
    }

    report[report_i++] = REP_LF;
    report[report_i++] = REP_CR;
    report[report_i++] = REP_EOL;
    
    send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
}