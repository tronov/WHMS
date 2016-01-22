#include "messages.h"

typedef struct {
    unsigned char message;
    void *vp_parameter;
} Message;

volatile Message messages_buffer[MESSAGES_NUMBER];

volatile unsigned char broadcast_messages_buffer[BROADCAST_MESSAGES_NUMBER];

void messages_init()
{
    unsigned char i;
    for (i = 0; i < MESSAGES_NUMBER; i++)
        messages_buffer[i].message = 0;
    for (i = 0; i < BROADCAST_MESSAGES_NUMBER; i++)
        broadcast_messages_buffer[i] = 0;
}

void messages_proc()
{
    unsigned char i;
    for (i = 0; i < MESSAGES_NUMBER; i++)
    {
        if (messages_buffer[i].message == 2)
            messages_buffer[i].message = 0;
        if (messages_buffer[i].message == 1)
            messages_buffer[i].message = 2;
    }
    for (i = 0; i < BROADCAST_MESSAGES_NUMBER; i++)
    {
        if (broadcast_messages_buffer[i] == 2)
            broadcast_messages_buffer[i] = 0;
        if (broadcast_messages_buffer[i] == 1)
            broadcast_messages_buffer[i] = 2;
    }
}

void send_message(unsigned char message)
{
    if(message > MESSAGES_NUMBER) return;
    if(messages_buffer[message].message == 0)
        messages_buffer[message].message = 1;
}

unsigned char get_message(unsigned char message)
{
    if(message > MESSAGES_NUMBER) return 0;
    
    if(messages_buffer[message].message == 2)
    {
        messages_buffer[message].message = 0;
        return 1;
    }
    return 0;
}

void send_message_w_param(unsigned char message, void *vp_parameter)
{
    if(messages_buffer[message].message == 0)
    {
        messages_buffer[message].message = 1;
        messages_buffer[message].vp_parameter = vp_parameter;
    }        
}

void *get_message_param(unsigned char message)
{
    return messages_buffer[message].vp_parameter;
}

void send_broadcast_message(unsigned char message)
{
    if(message > BROADCAST_MESSAGES_NUMBER) return;
    if(broadcast_messages_buffer[message] == 0)
        broadcast_messages_buffer[message] = 1;
}

unsigned char get_broadcast_message(unsigned char message)
{
    if(message > BROADCAST_MESSAGES_NUMBER) return 0;
    
    if(broadcast_messages_buffer[message] == 2) return 1;
    return 0;
}