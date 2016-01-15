#ifndef MESSAGES_H_
#define MESSAGES_H_

// Messages definitions
#define MSG_ADC_COMPLETE        0   // 
#define MSG_UART_TX_START       1   // ��������� ��� UART_TX: ������ ��������
#define MSG_UART_TX_COMPL       2   //
#define MSG_UART_RX_BYTE_RCV    3
#define MSG_UART_TX_REARY       4   // ��������� ��� CONTROL: UART ����� � �������� ������
#define MSG_CONTROL_CMD_RCV     5   // ��������� ��� CONTROL: ������� �������

#define MESSAGES_NUMBER         6


#define BROADCAST_MESSAGES_NUMBER   0


void messages_init(void);

void messages_proc(void);


void send_message(unsigned char message);

unsigned char get_message(unsigned char message);


void send_message_w_param(unsigned char message, unsigned char *vp_parameter);

void *get_message_param(unsigned char message);


void send_broadcast_message(unsigned char message);

unsigned char get_broadcast_message(unsigned char message);

#endif /* MESSAGES_H_ */