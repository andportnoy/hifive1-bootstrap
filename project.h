#include <stdint.h>
#include <stddef.h>

#define PRCIADDR 0x10008000
#define GPIOADDR 0x10012000
#define UARTADDR 0x10013000
#define PWM0ADDR 0x10015000
#define PWM1ADDR 0x10025000
#define PWM2ADDR 0x10035000

#define MTIMECMPADDR 0x2004000
#define MTIMEADDR    0x200bff8

#define MTIMECMPPTR ((u64 volatile *)0x2004000)
#define MTIMEPTR    ((u64 volatile *)0x200bff8)

#define BIT(i) (1<<(i))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "utils.h"
