#include <gctypes.h>
#include "retain_vars.h"
volatile u8 encryptionDirection __attribute__((section(".data"))) = 0;
volatile u8 logNSSLRead __attribute__((section(".data"))) = 0;
volatile u8 logNSSLWrite __attribute__((section(".data"))) = 0;
