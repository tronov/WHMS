#include "adc.h"
#include "messages.h"

#include <avr/io.h>
#include <stdlib.h>

double adc_data;

#define ADC_STATE_IDLE      0
#define ADC_STATE_GET_0     1
#define ADC_STATE_GET_1     2
#define ADC_STATE_GET_2     3

// Новое и предыдущее состояния
unsigned char adc_state, adc_state_prev;

void adc_init()
{
    ADCSRA = (1 << ADEN) | (3 << ADPS0) | (1 << ADSC);
    ADMUX = (1 << REFS0) | (1 << ADLAR) | (0 << MUX0);
    
    adc_state_prev  = ADC_STATE_IDLE;
    adc_state       = ADC_STATE_IDLE;
}

void adc_proc()
{
    switch (adc_state_prev)
    {
    case ADC_STATE_IDLE:
        if(get_message(MSG_ADC_GET_0))
            adc_state = ADC_STATE_GET_0;
        else if(get_message(MSG_ADC_GET_1))
            adc_state = ADC_STATE_GET_1;
        else if(get_message(MSG_ADC_GET_2))
            adc_state = ADC_STATE_GET_2;
        break;
    case ADC_STATE_GET_0:
    case ADC_STATE_GET_1:
    case ADC_STATE_GET_2:
        if (ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            adc_data = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            adc_state = ADC_STATE_IDLE;
            send_message_w_param(MSG_ADC_GET_OK, &adc_data);
        }
        break;
    default: break;
    }
    
    if (adc_state != adc_state_prev)
    {
        switch (adc_state)
        {
        case ADC_STATE_IDLE:
            //send_message(MSG_ADC_GET_OK);
            break;
        case ADC_STATE_GET_0:
            ADMUX = MUX0 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case ADC_STATE_GET_1:
            ADMUX = MUX1 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case ADC_STATE_GET_2:
            ADMUX = MUX2 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        default: break;
        }                
    }
    
    adc_state_prev = adc_state;
}
