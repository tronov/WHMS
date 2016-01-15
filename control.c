#include "control.h"
#include "timers.h"
#include "messages.h"
#include "signals.h"
#include "ports.h"
#include "adc.h"

#include <avr/io.h>
#include <stdlib.h>

// Состояния системы управления
#define CONTROL_STATE_IDLE	    0   // Простой системы управления
#define CONTROL_STATE_REPORT    1   // Передача состояния объектов управления
#define CONTROL_STATE_EXECUTE   2   // Исполнение принятой команды управления

// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Указатель для работы с параметром - массивом команды
unsigned char *cmd_ptr;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;


void control_init(void)
{   
    signals_init();
        
    control_state_prev  = CONTROL_STATE_IDLE;
    control_state       = CONTROL_STATE_IDLE;
    
    start_gtimer(GTIMER_CONTROL);
}

void control_proc(void)
{
    switch (control_state_prev)
    {
    case CONTROL_STATE_IDLE:
        if(get_message(MSG_CONTROL_CMD_RCV))
            control_state = CONTROL_STATE_EXECUTE;
        else if(get_gtimer(GTIMER_CONTROL) >= REP_TIMEOUT)
            control_state = CONTROL_STATE_REPORT;
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
            stop_gtimer(GTIMER_CONTROL);
            start_gtimer(GTIMER_CONTROL);
            break;
        case CONTROL_STATE_REPORT:
            // Обновление данных о входных сигналах
            //signals_proc();
			
            //report[0] = REP_START;
            
            {
                char * buffer = calloc(sizeof(char), 10);
                unsigned char buffer_i;
                
                unsigned char adc_i;
                
                for (adc_i = 0, report_i = 0; adc_i < ADC_NUMBER; adc_i++)
                {
                    dtostrf(get_adc(adc_i), 1, 5, buffer);
                    
                    for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
                    {
                        report[report_i++] = buffer[buffer_i++];
                    }
                    
                    report[report_i++] = REP_DELIM;
                }
                free(buffer);
                
                report[report_i++] = REP_LF;
                report[report_i++] = REP_CR;
                report[report_i++] = REP_EOL;
                
                send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
            }            
            break;
        case CONTROL_STATE_EXECUTE:        
            cmd_ptr = (unsigned char *)get_message_param(MSG_CONTROL_CMD_RCV);
            
            *(cmd_ptr)   ? set_signal(Uout) : clr_signal(Uout);
            *(cmd_ptr+1) ? set_signal(Rel)  : clr_signal(Rel);

            break;
        default: break;
        }
    }
    
    control_state_prev = control_state;
}