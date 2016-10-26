#pragma once

// ����� ������������� ������� ����
#define CONTROL_RUSHES_MAX      2

// ����������� ������ �������� ���� ������� ����������� �����-�������� �������������� (����� ����������������)
#define CONTROL_I_TG_ABS_MIN    0.01

// ��������������� �������� ������
#define REP_START   '>'
#define REP_DELIM   ' '
#define REP_LF      '\n'
#define REP_CR      '\r'
#define REP_EOL     '\0'
#define REP_SIZE    60U

// ���������� ��������� ����������
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

// ���������� �������
void send_report(double g);
struct Point3 get_derivative(struct Point2 p);
void reset_calculations(void);
unsigned char make_calculations(double u,double i);


