#ifndef SIGNALS_H_
#define SIGNALS_H_

typedef enum
{
    Uin =  0,
    Iin =  1,
    Gin =  2,
} input_signal; // Входные сигналы

#define INPUT_SIGNALS_NUMBER    3U


typedef enum
{
    Uout =  0,
    Rel  =  1,
} output_signal;// Исходящие сигналы

#define OUTPUT_SIGNALS_NUMBER   2U


void signals_init(void);

void signals_proc(void);

unsigned char get_signal(input_signal signal);

void set_signal(output_signal signal);

void clr_signal(output_signal signal);

#endif /* SIGNALS_H_ */