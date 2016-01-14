#ifndef UART_RX_H_
#define UART_RX_H_

#define COMMAND_SIZE    15
#define CHECKSUM_SIZE   2

#define RX_BUFFER_SIZE  32U
#define RX_BUFFER_MASK  (RX_BUFFER_SIZE - 1)

void uart_rx_init(void);

void uart_rx_proc(void);


#endif /* UART_RX_H_ */