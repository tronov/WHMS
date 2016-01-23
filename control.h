#ifndef CONTROL_H_
#define CONTROL_H_

#include "timer.h"

// Timeout in ticks (1 tick ~ 0.1 millisecond)
#define REP_TIMEOUT (1 * SEC)
#define PWM_TIMEOUT (5 * MS)



// Вспомогательные элементы отчета
#define REP_START   '>'
#define REP_DELIM   ' '
#define REP_LF      '\n'
#define REP_CR      '\r'
#define REP_EOL     '\0'
#define REP_SIZE    60U


void control_init(void);

void control_proc(void);


#endif /* CONTROL_H_ */