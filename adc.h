#ifndef ADC_H_
#define ADC_H_

#include "timers.h"

#define ADC_TIMEOUT     100 * MS
#define ADC_NUMBER      3
#define ADC_REFERENCE   5.0
#define ADC_RESOLUTION  65536

void adc_init(void);

void adc_proc(void);

double get_adc(unsigned char n);

#endif /* ADC_H_ */