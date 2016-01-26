#pragma once

#include "timer.h"

// Вспомогательные элементы отчета
#define REP_START   '>'
#define REP_DELIM   ' '
#define REP_LF      '\n'
#define REP_CR      '\r'
#define REP_EOL     '\0'
#define REP_SIZE    60U

void control_init(void);
void control_proc(void);
