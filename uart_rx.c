#include "uart_rx.h"

#include "messages.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define UART_RX_STATE_BEGIN     0   // ќжидание начала команды
#define UART_RX_STATE_CMD_HEAD  1   // Ќачало команды
#define UART_RX_STATE_CMD_BODY  2   // ѕрием тела команды
#define UART_RX_STATE_DELIM     3   // ѕрием разделител€
#define UART_RX_STATE_CS        4   // ѕрием контрольной суммы
#define UART_RX_STATE_CS_CHK    5   // ѕроверка контрольной суммы
#define UART_RX_STATE_END       6   //  оманда прин€та

// ¬спомогательные элементы команды
#define CMD_HEAD    '<'
#define CMD_DELIM   '='
#define CMD_END     '\r'


volatile unsigned char UART_RX_state;

volatile unsigned char data;

// ћассив значений прин€той команды и его индекс
volatile unsigned char command[COMMAND_SIZE];
volatile unsigned char command_i;

// ѕеременна€ контрольной суммы и индекс числа контрольной суммы
volatile unsigned char checksum;
volatile unsigned char checksum_i;


ISR(USART_RXC_vect)
{
    UCSRB &= ~(1<<RXCIE);   // запретить прерывание по приему байта
    
    // ѕрин€тый байт
    data = UDR;
    
    send_message(MSG_UART_RX_BYTE_RCV);
}



void uart_rx_init()
{
    cli();
    
    UCSRB |= (1<<RXCIE)   // разрешить прерывание по приему байта
           | (1<<RXEN);   // разрешить прием
    
    sei();
    
    UART_RX_state = UART_RX_STATE_BEGIN;
}

void uart_rx_proc()
{
	UCSRB &= ~(1<<RXCIE);   // запретить прерывание по приему байта
	
    unsigned char has_new_data = get_message(MSG_UART_RX_BYTE_RCV);
   
    switch(UART_RX_state)
    {
    case UART_RX_STATE_BEGIN:
		UCSRB |= (1<<RXCIE);   // разрешить прерывание по приему байта
        if(has_new_data && data == CMD_HEAD)
            UART_RX_state = UART_RX_STATE_CMD_HEAD;
        break;
    case UART_RX_STATE_CMD_HEAD:
        command_i = 0;
        UART_RX_state = UART_RX_STATE_CMD_BODY;
        break;
    case UART_RX_STATE_CMD_BODY:
		UCSRB |= (1<<RXCIE);   // разрешить прерывание по приему байта
	
        if(!has_new_data) UART_RX_state = UART_RX_STATE_CMD_BODY;
        else if(data == CMD_HEAD)
            UART_RX_state = UART_RX_STATE_CMD_HEAD;
        else
        {
            command[command_i] = (data - 48);
            command_i++;
            
            if(command_i < COMMAND_SIZE)
                UART_RX_state = UART_RX_STATE_CMD_BODY;
            else UART_RX_state = UART_RX_STATE_DELIM;
        }
        break;
    case UART_RX_STATE_DELIM:
		UCSRB |= (1<<RXCIE);   // разрешить прерывание по приему байта
		
        if(!has_new_data) UART_RX_state = UART_RX_STATE_DELIM;
        else if(data == CMD_HEAD)
            UART_RX_state = UART_RX_STATE_CMD_HEAD;
        else if(data != CMD_DELIM)
            UART_RX_state = UART_RX_STATE_BEGIN;
        else
        {
            checksum = 0;
            checksum_i = 0;
            UART_RX_state = UART_RX_STATE_CS;
        }
        break;
    case UART_RX_STATE_CS:
		UCSRB |= (1<<RXCIE);   // разрешить прерывание по приему байта
		
        if(!has_new_data) UART_RX_state = UART_RX_STATE_CS;
        else if(data == CMD_HEAD)
            UART_RX_state = UART_RX_STATE_CMD_HEAD;
        else
        {
            checksum = checksum * 10 + (data - 48);
            checksum_i++;
            
            if(checksum_i < CHECKSUM_SIZE)
                UART_RX_state = UART_RX_STATE_CS;
            else
            {
                command_i = 0;
                UART_RX_state = UART_RX_STATE_CS_CHK;
            }            
        }
        break;
    case UART_RX_STATE_CS_CHK:    
        checksum = checksum - command[command_i];
        command_i++;
        if(command_i < COMMAND_SIZE)
            UART_RX_state = UART_RX_STATE_CS_CHK;
        else if(checksum != 0)
            UART_RX_state = UART_RX_STATE_BEGIN;
        else UART_RX_state = UART_RX_STATE_END;
        break;
    case UART_RX_STATE_END:
		UCSRB |= (1<<RXCIE);   // разрешить прерывание по приему байта
		
        if(!has_new_data) UART_RX_state = UART_RX_STATE_END;
        else if(data == CMD_HEAD)
            UART_RX_state = UART_RX_STATE_CMD_HEAD;
        else if(data != CMD_END)
            UART_RX_state = UART_RX_STATE_BEGIN;
        else
		{
            send_message_w_param(MSG_CONTROL_CMD_RCV, (unsigned char *) &command);
            UART_RX_state = UART_RX_STATE_BEGIN;
		}
        break;
    default: break;
    }
}