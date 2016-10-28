#include "messages.h"

#include "calc.h"


#include <stdlib.h>



volatile struct Point3 control_report_data [CONTROL_RUSHES_MAX];
volatile unsigned char control_report_data_i = 0;

// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;


    volatile unsigned char iter;
    volatile double x1, x2, x3, x4, x5, h;
    volatile double y_1, y2, y3, y4, y5,
    d1y1, d1y2, d1y3, d1y4,
    d2y1, d2y2, d2y3,
    d3y1, d3y2,
    d4y1;
    volatile double y_derivative;


    volatile struct Point3 control_point3_prev = {0};
    volatile struct Point2 new_point = {0}, local_max = {0};
    volatile struct Point3 current = {0};
    volatile unsigned char is_in_rush;  // Флаг участка броска тока
    
    
    



struct Point3 get_derivative(struct Point2 p)
{
    volatile struct Point3 result = {0};

    if (x5 - p.X == 0) return result;
    
    x1 = x2;
    x2 = x3;
    x3 = x4;
    x4 = x5;
    x5 = p.X;

    y_1 = y2;
    y2 = y3;
    y3 = y4;
    y4 = y5;
    y5 = p.Y;
    
    d1y1 = d1y2;
    d1y2 = d1y3;
    d1y3 = d1y4;
    d1y4 = y5 - y4;
    
    d2y1 = d2y2;
    d2y2 = d2y3;
    d2y3 = d1y4 - d1y3;
    
    d3y1 = d3y2;
    d3y2 = d2y3 - d2y2;
    
    d4y1 = d3y2 - d3y1;
    
    // Функция возвращает точку с начала буфера
    // Восемь итераций на заполнение буфера точек и дельт
    if (iter++ < 8) return result;
    
    h = ((x5 - x4) + (x4 - x3) + (x3 - x2) + (x2 - x1)) / 4;
    
    y_derivative = (1 / (h * h)) * (d1y1 - (d2y1 / 2) + (d3y1 / 3) - (d4y1 / 4));

    result.X = x1;
    result.Y = y_1;
    result.Z = y_derivative;
    
    return result;
}

void reset_calculations()
{
    control_point3_prev.X = 0.0;
    control_point3_prev.Y = 0.0;
    control_point3_prev.Z = 0.0;
    
    new_point.X = 0.0;
    new_point.Y = 0.0;
    
    local_max.X = 0.0;
    local_max.Y = 0.0;        
        
    current.X = 0.0;
    current.Y = 0.0;
    current.Z = 0.0;
    
    is_in_rush = 0;
    
    iter = 0;
    x1 = x2 = x3 = x4 = x5 = h = 0.0;
    y_1 = y2 = y3 = y4 = y5 = 
        d1y1 = d1y2 = d1y3 = d1y4 = 
        d2y1 = d2y2 = d2y3 = 
        d3y1 = d3y2 = 
        d4y1 = 0.0;
    y_derivative = 0.0;
    
    control_report_data_i = 0;
}

unsigned char make_calculations(double u,double i)
{
    new_point.X = u;
    new_point.Y = i;
    
    current = get_derivative(new_point);
    
    if (current.X == 0 && current.Y == 0 && current.Z == 0)
        return control_report_data_i;;
    
    if (control_point3_prev.X == 0 && control_point3_prev.Y == 0 && control_point3_prev.Z == 0)
    {
        control_point3_prev = current;
        return control_report_data_i;
    }
    if (current.X - control_point3_prev.X == 0)
        return control_report_data_i;
    
    
    volatile double i_tan = (current.Y - control_point3_prev.Y) / (current.X - control_point3_prev.X);
    
    send_report(current.Z);
    
    // --- fabs(i_tan)
    if (i_tan > CONTROL_I_TG_ABS_MIN)
    // Если текущая точка находится на участке броска тока
    {
        if (!is_in_rush)
        // Если текущая точка первая на участке броска тока
        {
            is_in_rush = 1;
            control_report_data[control_report_data_i].X = current.Y;
        }
        else
        {
            if (local_max.Y < current.Z)
            {
                local_max.Y = current.Z;
                local_max.X = current.X;
            }
        }
    }
    else
    // Если текущая точка не находится на участке броска тока
    {
        if (is_in_rush)
        // Если текущая точка первая после участка броска тока
        {
            control_report_data[control_report_data_i].Y = local_max.X;
            control_report_data[control_report_data_i].Z = current.Y;
            control_report_data_i++;
            
            is_in_rush = 0;
            local_max.X = 0.0;
            local_max.Y = 0.0;
        }
    }
    
    control_point3_prev = current;
    return control_report_data_i;
}

void send_report(double g)
{
    char buffer [10];
    unsigned char buffer_i;
    
    report_i = 0;
    
    dtostrf( g, 1, 5, (char *) &buffer);
    
    for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
    {
        report[report_i++] = buffer[buffer_i++];
    }

    report[report_i++] = REP_DELIM;
    
    volatile unsigned char i;
    for (i = 0; i < control_report_data_i; i++)
    {
        dtostrf( control_report_data[i].Y, 1, 5, (char *) &buffer);
        
        for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
        {
            report[report_i++] = buffer[buffer_i++];
        }
        
        report[report_i++] = REP_DELIM;
        
        
        volatile double delta_i = control_report_data[i].Z - control_report_data[i].X;
        dtostrf( delta_i, 1, 5, (char *) &buffer);
        
        for (buffer_i = 0; buffer[buffer_i] != REP_EOL;)
        {
            report[report_i++] = buffer[buffer_i++];
        }
        
        report[report_i++] = REP_DELIM;
    }
    
    control_report_data_i = 0;    // Сброс индекса для разблокировки приращения частоты ШИМ

    report[report_i]   = REP_LF;  // Подмена последнего символа REP_DELIM
    report[report_i++] = REP_CR;
    report[report_i++] = REP_EOL;
    
    send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
}