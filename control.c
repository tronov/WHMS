#include "control.h"
#include "timers.h"
#include "messages.h"
#include "ports.h"
#include "adc.h"

#include <avr/io.h>
#include <stdlib.h>
#include <math.h>

// Состояния системы управления
#define CONTROL_STATE_IDLE	        0   // Простой системы управления
#define CONTROL_STATE_R_ON          1   // Включение реле
#define CONTROL_STATE_R_ON_TIMEOUT  2   // Тайм-аут переходных процессов после включения реле
#define CONTROL_STATE_GET_G         3   // Измерение проводимости (Gin)
#define CONTROL_STATE_R_OFF         4   // Выключение реле
#define CONTROL_STATE_R_OFF_TIMEOUT 5   // Тайм-аут переходных процессов после выключения реле
#define CONTROL_STATE_SET_PWM       6   // Установка частоты ШИМ
#define CONTROL_STATE_PWM_TIMEOUT   7   // Тайм-аут переходных процессов после изменения частоты ШИМ
#define CONTROL_STATE_GET_U         8   // Измерение потенциала катода полярографической ячейки (Uin)
#define CONTROL_STATE_GET_I         9   // Измерение тока полярографической ячейки (Iin)
#define CONTROL_STATE_CALCULATE     10  // Анализ считанных данных
#define CONTROL_STATE_RESET_PWM     11  // Сброс частоты ШИМ на минимальное значение
#define CONTROL_STATE_REPORT        12  // Отправка отчёта по UART


// Новое и предыдущее состояния системы управления
unsigned char control_state, control_state_prev;

// Объявление приватных констант
#define CONTROL_PWM_MIN         0
#define CONTROL_PWM_MAX         255
// Минимальный модуль тангенса угла наклона касательной вольт-амперной характеристики (порог чувствительности)
#define CONTROL_I_TG_ABS_MIN    0.02
// Timeout in ticks (1 tick ~ 0.1 millisecond)
#define CONTROL_TIMEOUT         (1 * SEC)   // Тайм-аут запуска измерений
#define CONTROL_TIMEOUT_R_ON    (100 * MS)  // Тайм-аут переходных процессов после подачи напряжения на реле
#define CONTROL_TIMEOUT_R_OFF   (100 * MS)  // Тайм-аут переходных процессов после измерения напряжения Gin
#define CONTROL_TIMEOUT_PWM     (5 * MS)    // Тайм-аут переходных процессов после изменения частоты ШИМ
// Число отслеживаемых бросков тока
#define CONTROL_RUSHES_MAX      2


// Объявление приватных переменных
struct Point2
{
    volatile double X;
    volatile double Y;
};
struct Point3
{
    volatile double X;
    volatile double Y;
    volatile double Z;
};
unsigned int control_pwm;
volatile double control_g,
                control_u,
                control_i;

volatile struct Point3 control_report_data [CONTROL_RUSHES_MAX];
volatile unsigned char control_report_data_i = 0;

// Объявление приватных функций
void send_report(void);
struct Point3 get_derivative(struct Point2 p);
void make_calculations(void);
void debug(unsigned char *message);


// Массив отчета и его индекс
unsigned char report[REP_SIZE];
volatile unsigned char report_i;





//////////////////////////////////////////////////////////////////////////
// get_derivative
volatile unsigned char iter = 0;
volatile double x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, h = 0;
volatile double   y_1 = 0,   y2 = 0,   y3 = 0,   y4 = 0,   y5 = 0,
                d1y1 = 0, d1y2 = 0, d1y3 = 0, d1y4 = 0,
                d2y1 = 0, d2y2 = 0, d2y3 = 0,
                d3y1 = 0, d3y2 = 0,
                d4y1 = 0;
volatile double y_derivative = 0;
volatile struct Point3 result;

// make_calculations
volatile struct Point3 control_point3_prev = {0.0, 0.0, 0.0};
volatile unsigned char is_in_rush = 0;  // Флаг участка броска тока
volatile struct Point2 new_point, local_max = {0.0, 0.0};
volatile struct Point3 current;
volatile double i_tan;

//////////////////////////////////////////////////////////////////////////





void control_init(void)
{
    ports_configure();
    
    control_pwm = CONTROL_PWM_MIN;
    
    control_state_prev  = CONTROL_STATE_R_ON;
    control_state = control_state_prev;
    
    start_gtimer(GTIMER_CONTROL);
}

void control_proc(void)
{
    switch (control_state_prev)
    {
    case CONTROL_STATE_IDLE:
        if (get_gtimer(GTIMER_CONTROL) >= CONTROL_TIMEOUT)
            control_state = CONTROL_STATE_R_ON;
        break;
    case CONTROL_STATE_R_ON:
        control_state = CONTROL_STATE_R_ON_TIMEOUT;
        break;
    case CONTROL_STATE_R_ON_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_R_ON) > CONTROL_TIMEOUT_R_ON)
            control_state = CONTROL_STATE_GET_G;
        break;
    case CONTROL_STATE_GET_G:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_g = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_R_OFF;
        }
        break;
    case CONTROL_STATE_R_OFF:
        control_state = CONTROL_STATE_R_OFF_TIMEOUT;
        break;
    case CONTROL_STATE_R_OFF_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_R_OFF) > CONTROL_TIMEOUT_R_OFF)
            control_state = CONTROL_STATE_SET_PWM;
        break;
    case CONTROL_STATE_SET_PWM:
        if (get_message(MSG_PWM_SET_OK))
            control_state = CONTROL_STATE_PWM_TIMEOUT;
        else if (get_message(MSG_PWM_SET_ERR))
            control_state = CONTROL_STATE_RESET_PWM;
        break;
    case CONTROL_STATE_PWM_TIMEOUT:
        if (get_gtimer(GTIMER_CONTROL_PWM) > CONTROL_TIMEOUT_PWM)
            control_state = CONTROL_STATE_GET_U;
        break;
    case CONTROL_STATE_GET_U:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_u = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_GET_I;
        }
        break;
    case CONTROL_STATE_GET_I:
        if (get_message(MSG_ADC_GET_OK))
        {
            control_i = *((double *) get_message_param(MSG_ADC_GET_OK));
            control_state = CONTROL_STATE_CALCULATE;
        }
        break;
    case CONTROL_STATE_CALCULATE:
        if (control_report_data_i >= CONTROL_RUSHES_MAX)
            control_state = CONTROL_STATE_RESET_PWM;
        else if (control_pwm < CONTROL_PWM_MAX)
            control_state = CONTROL_STATE_SET_PWM;
        else
            control_state = CONTROL_STATE_RESET_PWM;
        break;
    case CONTROL_STATE_RESET_PWM:
        if (get_message(MSG_PWM_SET_OK) || get_message(MSG_PWM_SET_ERR))
            control_state = CONTROL_STATE_REPORT;
        break;
    case CONTROL_STATE_REPORT:
        control_state = CONTROL_STATE_IDLE;
        break;
    default: break;
    }
    
    if (control_state != control_state_prev)
    {
        switch (control_state)
        {
        case CONTROL_STATE_IDLE:
            stop_gtimer(GTIMER_CONTROL);
            start_gtimer(GTIMER_CONTROL);
            break;
        case CONTROL_STATE_R_ON:
            Rel_PORT |= (1 << Rel_PORT_N);
            break;
        case CONTROL_STATE_R_ON_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_R_ON);
            start_gtimer(GTIMER_CONTROL_R_ON);
            break;
        case CONTROL_STATE_GET_G:
            send_message(MSG_ADC_GET_2);
            break;
        case CONTROL_STATE_R_OFF:
            Rel_PORT &= ~(1 << Rel_PORT_N);
            break;
        case CONTROL_STATE_R_OFF_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_R_OFF);
            start_gtimer(GTIMER_CONTROL_R_OFF);
            break;
        case CONTROL_STATE_SET_PWM:
            control_pwm++;
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_PWM_TIMEOUT:
            stop_gtimer(GTIMER_CONTROL_PWM);
            start_gtimer(GTIMER_CONTROL_PWM);
            break;
        case CONTROL_STATE_GET_U:
            send_message(MSG_ADC_GET_0);
            break;
        case CONTROL_STATE_GET_I:
            send_message(MSG_ADC_GET_1);
            break;
        case CONTROL_STATE_CALCULATE:
            if (control_report_data_i < CONTROL_RUSHES_MAX)
                make_calculations();
            break;
        case CONTROL_STATE_RESET_PWM:
            control_pwm = CONTROL_PWM_MIN;
            send_message_w_param(MSG_PWM_SET, &control_pwm);
            break;
        case CONTROL_STATE_REPORT:
            send_report();
            break;
        default: break;
        }
        
        control_state_prev = control_state;
    }
}


void send_report()
{
    char buffer [10];
    unsigned char buffer_i;
    
    report_i = 0;
    
    dtostrf( control_g, 1, 5, (char *) &buffer);
        
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
        
        
        volatile double delta_i = control_report_data[i].X - control_report_data[i].Z;        
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

struct Point3 get_derivative(struct Point2 p)
{
    //result.X = x1;
    //result.Y = y_1;
    //result.Z = y_derivative;

    result.X = 0.0;
    result.Y = 0.0;
    result.Z = 0.0;

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

void make_calculations()
{
    new_point.X = control_u;
    new_point.Y = control_i;
    
    current = get_derivative(new_point);
    
    if (current.X == 0 && current.Y == 0 && current.Z == 0) return;
    
    if (control_point3_prev.X == 0 && control_point3_prev.Y == 0 && control_point3_prev.Z == 0)
    {
        control_point3_prev = current;
        return;
    }        
    if (current.X - control_point3_prev.X == 0) return;
    
    i_tan = (current.Y - control_point3_prev.Y) / (current.X - control_point3_prev.X);
    
    
    if (fabs(i_tan) > CONTROL_I_TG_ABS_MIN)
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
}

void debug(unsigned char *message)
{
    for (report_i = 0; *message != REP_EOL && report_i < REP_SIZE; message++)
    {
        report[report_i++] = *message;
    }
    
    report[report_i++] = REP_LF;
    report[report_i++] = REP_CR;
    report[report_i++] = REP_EOL;
    
    send_message_w_param(MSG_UART_TX_START, (unsigned char *) &report);
}