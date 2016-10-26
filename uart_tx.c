 #include "uart_tx.h"
 //
 //#include "control.h"
 
 #include "messages.h"
 
 #include <avr/io.h>
 #include <avr/interrupt.h>
 
 
 #define UART_TX_STATE_IDLE     0   // ��������
 #define UART_TX_STATE_TR       1   // ��������
 #define UART_TX_STATE_TR_END   2   // �������� ��������
 
 
 unsigned char UART_TX_state, UART_TX_state_prev;
 
 volatile unsigned char uart_tx_free;
 
 
 // ��������� ������ �����������
 struct {
     unsigned char buffer[TX_BUFFER_SIZE];   // ����� �����������
     volatile unsigned char head;            // ������ ������ ������
     volatile unsigned char tail;            // ������ ����� ������
 } TX;
 
 // ������� ������� ������ �����������
 inline void clr_TX(void)
 {
     TX.tail = TX.head = 0;
 }


// ��������� ��� ������ � ���������� - ������� ������� ����������
volatile unsigned char *rep_ptr;
 
 
 ISR(USART_UDRE_vect)
 {
    if(TX.head != TX.tail)
    {
        UDR = TX.buffer[TX.head];
        TX.head = (TX.head + 1) & TX_BUFFER_MASK;
    }
    else
    {
        UCSRB &= ~(1<<UDRIE); // ��������� ���������� �� ����������� ��������
        UCSRB |=  (1<<TXCIE); // ��������� ���������� �� ��������� ��������
    }
 }
 
ISR(USART_TXC_vect)
{
    uart_tx_free = 1;
} 
 
 void uart_tx_init()
{
    cli();
    
    UCSRB |=  (1<<TXEN);  // ��������� ��������
    UCSRB &= ~(1<<UDRIE); // ��������� ���������� �� ����������� ��������
    UCSRB &= ~(1<<TXCIE); // ��������� ���������� �� ��������� �������� �����
    sei();    
    
    clr_TX();
    
    uart_tx_free = 1;
    
    UART_TX_state_prev = UART_TX_STATE_IDLE;
 }
 
 
 void uart_tx_proc()
 {
    switch(UART_TX_state_prev)
    {
    case UART_TX_STATE_IDLE:
       if(get_message(MSG_UART_TX_START))
           UART_TX_state = UART_TX_STATE_TR;
       break;
    case UART_TX_STATE_TR:
        if(uart_tx_free)
            UART_TX_state = UART_TX_STATE_TR_END;
        break;       
    case UART_TX_STATE_TR_END:
       UART_TX_state = UART_TX_STATE_IDLE;
       break;
    default: break;
    }
     
    if(UART_TX_state != UART_TX_state_prev)
    {
        switch(UART_TX_state)
        {
        case UART_TX_STATE_IDLE:
            break;
        case UART_TX_STATE_TR:
            clr_TX();
            
            rep_ptr = get_message_param(MSG_UART_TX_START);
            do
            {
                TX.buffer[TX.tail] = *rep_ptr;
                TX.tail = (TX.tail + 1) & TX_BUFFER_MASK;
                rep_ptr++;
            } while (*rep_ptr != '\0');
        
            if(TX.tail != TX.head)
            {
                // ����������� �������� �������
                unsigned char temp = SREG;
                
                // ���������� ������ ����������
                cli();
                
                uart_tx_free = 0;
                
                // �������� ����������� ��������
                while(!(UCSRA & (1<<UDRE)));
                
                // ������ �������� ����� ������ � �������
                UDR = TX.buffer[TX.head];
                
                // ��������� ������� ������ ������
                TX.head = (TX.head + 1) & TX_BUFFER_MASK;
                
                // ���������� ���������� �� ����������� ��������
                UCSRB|= (1<<UDRIE);
                
                // �������������� �������� �������
                SREG = temp;
            }
            break;
        case UART_TX_STATE_TR_END:
            // ������ ���������� �� �����������
            UCSRB &= ~(1<<TXCIE);
            clr_TX();
            break;
        default: break;
        }
    }
    
    UART_TX_state_prev = UART_TX_state;
 }
 