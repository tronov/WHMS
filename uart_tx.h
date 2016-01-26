#pragma once

#define TX_BUFFER_SIZE  64U
#define TX_BUFFER_MASK  (TX_BUFFER_SIZE - 1)

void uart_tx_init(void);
void uart_tx_proc(void);
