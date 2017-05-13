#include <gctypes.h>
#include "retain_vars.h"
volatile u8 shouldLog __attribute__((section(".data"))) = 0;
