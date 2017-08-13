#include <gctypes.h>
#include "common/c_retain_vars.h"
u8 gAppStatus __attribute__((section(".data"))) = 0;
OSThread diiServer_thread __attribute__((section(".data")));
OSMessageQueue diiServer_serverQueue __attribute__((section(".data")));
OSMessageQueue diiServer_clientQueue __attribute__((section(".data")));
OSMessageQueue diiServer_fileQueue __attribute__((section(".data")));
OSMessage diiServer_serverMessages[MESSAGE_COUNT] __attribute__((section(".data")));
OSMessage diiServer_clientMessages[MESSAGE_COUNT] __attribute__((section(".data")));
OSMessage diiServer_fileMessage __attribute__((section(".data")));
OSContext diiServer_crashContext __attribute__((section(".data")));
u32 diiServer_crashType __attribute__((section(".data")));

u8 diiServer_crashState __attribute__((section(".data")));
u8 diiServer_stepState __attribute__((section(".data")));
u32 diiServer_stepSource __attribute__((section(".data")));

//10 general breakpoints + 2 step breakpoints
breakpoint diiServer_breakpoints[12] __attribute__((section(".data")));

bool diiServer_connection __attribute__((section(".data")));

FSCmdBlock diiServer_fileBlock __attribute__((section(".data")));
FSClient diiServer_fileClient __attribute__((section(".data")));
char * diiServer_patchFiles __attribute__((section(".data")));
int diiServer_fileHandles[NUM_FILE_HANDLES] __attribute__((section(".data")));

char diiServer_stack[STACK_SIZE] __attribute__((section(".data")));

CThread * diiServer_threadPointer  __attribute__((section(".data"))) = 0;
