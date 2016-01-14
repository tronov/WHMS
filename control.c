#include "control.h"
#include "timers.h"
#include "messages.h"
#include "signals.h"
#include "ports.h"

#include <avr/io.h>
#include <stdlib.h>

// Состояния системы управления
#define CONTROL_STATE_IDLE	    0   // Простой системы управления
#define CONTROL_READ_ADC0       1   //
#define CONTROL_READ_ADC1       2   //
#define CONTROL_READ_ADC2       3   //
#define CONTROL_STATE_REPORT    4   // Передача состояния объектов управления
#define CONTROL_STATE_EXECUTE   5   // Исполнение принятой команды управления

// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Указатель для работы с параметром - массивом команды
unsigned char *cmd_ptr;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;

#define ADC_REFERENCE 5.0
#define ADC_RESOLUTION 65536
char * adc0_data;
char * adc1_data;
char * adc2_data;
volatile unsigned char adc_data_i;

void control_init(void)
{   
    signals_init();

    adc0_data = calloc(10, sizeof(char));
    adc1_data = calloc(10, sizeof(char));
    adc2_data = calloc(10, sizeof(char));

    // TODO: Move to signals.c
    ADCSRA = (1 << ADEN) | (3 << ADPS0) | (1 << ADSC);
    ADMUX = (1<<REFS0)|(1<<ADLAR)|(0<<MUX0);
        
    control_state_prev = CONTROL_READ_ADC0;
    
    start_gtimer(GTIMER_CONTROL);
}

void control_proc(void)
{
    switch(control_state_prev)
    {
    case CONTROL_STATE_IDLE:
        if(get_message(MSG_CONTROL_CMD_RCV))
            control_state = CONTROL_STATE_EXECUTE;
        else if(get_gtimer(GTIMER_CONTROL) >= REP_TIMEOUT)
            control_state = CONTROL_STATE_REPORT;
        break;
    case CONTROL_READ_ADC0:
        if(ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            double data = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            dtostrf(data, 1, 5, adc0_data);
            control_state = CONTROL_READ_ADC1;
        }
        else control_state = CONTROL_READ_ADC0;
        break;
    case CONTROL_READ_ADC1:
        if(ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            double data = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            dtostrf(data, 1, 5, adc1_data);
            control_state = CONTROL_READ_ADC2;
        }
        else control_state = CONTROL_READ_ADC1;
        break;
    case CONTROL_READ_ADC2:
        if(ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            double data = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            dtostrf(data, 1, 5, adc2_data);
            control_state = CONTROL_STATE_IDLE;
        }
        else control_state = CONTROL_READ_ADC2;
        break;
    case CONTROL_STATE_REPORT:
        control_state = CONTROL_READ_ADC0;
        break;
    case CONTROL_STATE_EXECUTE:
        control_state = CONTROL_STATE_REPORT;
        break;
    default: break;
    }
    
    if(control_state != control_state_prev)
    {
        switch(control_state)
        {
        case CONTROL_STATE_IDLE:
            break;
        case CONTROL_READ_ADC0:
            ADMUX = MUX0 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case CONTROL_READ_ADC1:
            ADMUX = MUX1 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case CONTROL_READ_ADC2:
            ADMUX = MUX2 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case CONTROL_STATE_REPORT:
            // Обновление данных о входных сигналах
            //signals_proc();
			
            //report[0] = REP_START;
            
            for(report_i = 0, adc_data_i = 0; adc0_data[adc_data_i] != '\0'; report_i++, adc_data_i++)
                report[report_i] = adc0_data[adc_data_i];
            
            report[report_i++] = REP_DELIM;
            
            for(adc_data_i = 0; adc1_data[adc_data_i] != '\0'; report_i++, adc_data_i++)
                report[report_i] = adc1_data[adc_data_i];

            report[report_i++] = REP_DELIM;

            for(adc_data_i = 0; adc2_data[adc_data_i] != '\0'; report_i++, adc_data_i++)
                report[report_i] = adc2_data[adc_data_i];

            report[report_i++] = REP_LF;
            report[report_i++] = REP_CR;
            report[report_i++] = REP_EOL;

            send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
            
            // Сброс таймера отсчета периода посылки отчета
            stop_gtimer(GTIMER_CONTROL);
            start_gtimer(GTIMER_CONTROL);
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