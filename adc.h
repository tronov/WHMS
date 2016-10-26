#pragma once

#include "timers.h"

#define ADC_TIMEOUT     100 * MS

#define ADC_NUMBER      3
#define ADC_REFERENCE   5.0
#define ADC_RESOLUTION  65535//1024

void adc_init(void);
void adc_proc(void);