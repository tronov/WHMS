#ifndef MESSAGES_H_
#define MESSAGES_H_

// Messages definitions
#define MSG_ADC_GET_0           0   // ������� �� ������ �������� ������ ���
#define MSG_ADC_GET_1           1   // ������� �� ������ ������� ������ ���
#define MSG_ADC_GET_2           2   // ������� �� ������ ������� ������ ���
#define MSG_ADC_GET_OK          3   // ����� � ���������� ���

#define MSG_PWM_SET             4
#define MSG_PWM_SET_ERR         5
#define MSG_PWM_SET_OK          6

#define MSG_UART_TX_START       7   // ��������� ��� UART_TX: ������ ��������
#define MSG_UART_TX_COMPL       8   //
#define MSG_UART_RX_BYTE_RCV    9
#define MSG_UART_TX_REARY       10   // ��������� ��� CONTROL: UART ����� � �������� ������
#define MSG_CONTROL_CMD_RCV     11  // ��������� ��� CONTROL: ������� �������

#define MESSAGES_NUMBER         12

#define BROADCAST_MESSAGES_NUMBER   0


void messages_init(void);

void messages_proc(void);


void send_message(unsigned char message);

unsigned char get_message(unsigned char message);


void send_message_w_param(unsigned char message, void *vp_parameter);

void *get_message_param(unsigned char message);


void send_broadcast_message(unsigned char message);

unsigned char get_broadcast_message(unsigned char message);

#endif /* MESSAGES_H_ */