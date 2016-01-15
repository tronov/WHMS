#include "adc.h"
#include "messages.h"

#include <avr/io.h>
#include <stdlib.h>

double adc_data [ADC_NUMBER];

#define ADC_STATE_IDLE          0
#define ADC_STATE_READ_ADC0     1
#define ADC_STATE_READ_ADC1     2
#define ADC_STATE_READ_ADC2     3

// Новое и предыдущее состояния системы управления
unsigned char adc_state, adc_state_prev;

void adc_init()
{
    ADCSRA = (1 << ADEN) | (3 << ADPS0) | (1 << ADSC);
    ADMUX = (1 << REFS0) | (1 << ADLAR) | (0 << MUX0);
    
    adc_state_prev  = ADC_STATE_IDLE;
    adc_state       = ADC_STATE_IDLE;
    
    start_gtimer(GTIMER_ADC);
}

void adc_proc()
{
    switch (adc_state_prev)
    {
    case ADC_STATE_IDLE:
        if(get_gtimer(GTIMER_ADC) >= ADC_TIMEOUT)
            adc_state = ADC_STATE_READ_ADC0;
    break;
        break;
    case ADC_STATE_READ_ADC0:
        if (ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            // TODO: check for int truncate
            adc_data[0] = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            adc_state = ADC_STATE_READ_ADC1;
        }
        break;
    case ADC_STATE_READ_ADC1:
        if (ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            // TODO: check for int truncate
            adc_data[1] = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            adc_state = ADC_STATE_READ_ADC2;
        }
        break;
    case ADC_STATE_READ_ADC2:
        if (ADCSRA & (1 << ADIF)) // Conversion is complete
        {
            // TODO: check for int truncate
            adc_data[2] = ADC_REFERENCE / ADC_RESOLUTION * ADC;
            adc_state = ADC_STATE_IDLE;
        }
        break;
    default: break;
    }
    
    if (adc_state != adc_state_prev)
    {
        switch (adc_state)
        {
        case ADC_STATE_IDLE:
            stop_gtimer(GTIMER_ADC);
            start_gtimer(GTIMER_ADC);
            break;
        case ADC_STATE_READ_ADC0:
            ADMUX = MUX0 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case ADC_STATE_READ_ADC1:
            ADMUX = MUX1 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        case ADC_STATE_READ_ADC2:
            ADMUX = MUX2 | (ADMUX & 0xF0);
            ADCSRA |= (1 << ADSC);
            break;
        default: break;
        }                
    }
    
    adc_state_prev = adc_state;
}

double get_adc(unsigned char n)
{
    if (n >= ADC_NUMBER) return 0;
    else return adc_data[n];
}