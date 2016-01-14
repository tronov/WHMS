#include <avr/io.h>
#include "signals.h"

#ifndef PORTS_H_
#define PORTS_H_

// Input ports
#define Uin_DDR      DDRA
#define Uin_PORT     PORTA
#define Uin_PIN      PINA
#define Uin_PORT_N   PA0
#define Uin_PIN_N    PINA0

#define Iin_DDR      DDRA
#define Iin_PORT     PORTA
#define Iin_PIN      PINA
#define Iin_PORT_N   PA1
#define Iin_PIN_N    PINA1

#define Gin_DDR      DDRA
#define Gin_PORT     PORTA
#define Gin_PIN      PINA
#define Gin_PORT_N   PA2
#define Gin_PIN_N    PINA2

// Output ports
#define Uout_DDR     DDRB
#define Uout_PORT    PORTB
#define Uout_PIN     PINB
#define Uout_PORT_N  PB0
#define Uout_PIN_N   PINB0

#define Rel_DDR      DDRB
#define Rel_PORT     PORTB
#define Rel_PIN      PINB
#define Rel_PORT_N   PB1
#define Rel_PIN_N    PINB1

void ports_configure(void);

#endif /* PORTS_H_ */