 #include "uart_tx.h"
 //
 //#include "control.h"
 
 #include "messages.h"
 
 #include <avr/io.h>
 #include <avr/interrupt.h>
 
 
 #define UART_TX_STATE_IDLE     0   // Ожидание
 #define UART_TX_STATE_TR       1   // Передача
 #define UART_TX_STATE_TR_END   2   // Передача окончена
 
 
 unsigned char UART_TX_state, UART_TX_state_prev;
 
 volatile unsigned char uart_tx_free;
 
 
 // Структура буфера передатчика
 struct {
     unsigned char buffer[TX_BUFFER_SIZE];   // Буфер передатчика
     volatile unsigned char head;            // Индекс начала буфера
     volatile unsigned char tail;            // Индекс конца буфера
 } TX;
 
 // Функция очистки буфера передатчика
 inline void clr_TX(void)
 {
     TX.tail = TX.head = 0;
 }


// Указатель для работы с параметром - отчетом системы управления
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
        UCSRB &= ~(1<<UDRIE); // Запретить прерывание по опустошению регистра
        UCSRB |=  (1<<TXCIE); // Разрешить прерывание по окончанию передачи
    }
 }
 
ISR(USART_TXC_vect)
{
    uart_tx_free = 1;
} 
 
 void uart_tx_init()
{
    cli();
    
    UCSRB |=  (1<<TXEN);  // разрешить передачу
    UCSRB &= ~(1<<UDRIE); // запретить прерывание по опустошению регистра
    UCSRB &= ~(1<<TXCIE); // запретить прерывание по окончанию передачи байта
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
                // Копирование регистра статуса
                unsigned char temp = SREG;
                
                // Глобальный запрет прерываний
                cli();
                
                uart_tx_free = 0;
                
                // Ожидание опустошения регистра
                while(!(UCSRA & (1<<UDRE)));
                
                // Запись текущего байта буфера в регистр
                UDR = TX.buffer[TX.head];
                
                // Инкремент индекса начала буфера
                TX.head = (TX.head + 1) & TX_BUFFER_MASK;
                
                // Разрешение прерывания по опустошению регистра
                UCSRB|= (1<<UDRIE);
                
                // Восстановление регистра статуса
                SREG = temp;
            }
            break;
        case UART_TX_STATE_TR_END:
            // Запрет прерываний от передатчика
            UCSRB &= ~(1<<TXCIE);
            clr_TX();
            break;
        default: break;
        }
    }
    
    UART_TX_state_prev = UART_TX_state;
 }
 