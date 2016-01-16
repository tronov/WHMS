#ifndef PORTS_H_
#define PORTS_H_

#include <avr/io.h>

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
#define Uout_DDR     DDRD
#define Uout_PORT    PORTD
#define Uout_PIN     PIND
#define Uout_PORT_N  PD5
#define Uout_PIN_N   PIND5

#define Rel_DDR      DDRD
#define Rel_PORT     PORTD
#define Rel_PIN      PIND
#define Rel_PORT_N   PD4
#define Rel_PIN_N    PIND4

void ports_configure(void);

#endif /* PORTS_H_ */