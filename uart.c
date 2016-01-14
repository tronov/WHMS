#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "messages.h"

#include "uart_rx.h"
#include "uart_tx.h"

#include "control.h"

#define F_CPU   8000000UL
#define BAUD    9600
#define SETUBRR ((F_CPU)/16/(BAUD)-1)


void uart_init()
{
    cli();
    // ��������� ������������ ����������� ����������
    //CLKPR = 0x80;
    //CLKPR = 0;
    
    // ��������� �������� ������ �����
    UBRRH = (unsigned char)(SETUBRR>>8);
    UBRRL = (unsigned char)(SETUBRR);
            
    UCSRC = (1<<UCSZ1)|(1<<UCSZ0);   // ������ ��������� 8-n-1
        
    sei();
    
    uart_rx_init();
    uart_tx_init();
}

void uart_proc()
{
    uart_rx_proc();
    uart_tx_proc();
}

