#include <stdint.h>

#define GPIOADDR 0x10012000
#define UARTADDR 0x10013000

#define BIT(i) (1<<i)

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

#include "utils.h"
