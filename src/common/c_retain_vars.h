#ifndef C_RETAINS_VARS_H_
#define C_RETAINS_VARS_H_
#include "system/OSThread.h"
#include "common/diibugger_defs.h"
#include "dynamic_libs/os_types.h"
#include "dynamic_libs/fs_defs.h"

extern u8 gAppStatus;
extern CThread * diiServer_threadPointer;
extern OSThread diiServer_thread;
extern OSMessageQueue diiServer_serverQueue;
extern OSMessageQueue diiServer_clientQueue;
extern OSMessageQueue diiServer_fileQueue;
extern OSMessage diiServer_serverMessages[MESSAGE_COUNT];
extern OSMessage diiServer_clientMessages[MESSAGE_COUNT];
extern OSMessage diiServer_fileMessage;
extern OSContext diiServer_crashContext;
extern u32 diiServer_crashType;

extern u8 diiServer_crashState;
extern u8 diiServer_stepState;
extern u32 diiServer_stepSource;

//10 general breakpoints + 2 step breakpoints
extern breakpoint diiServer_breakpoints[12];

extern bool diiServer_connection;

extern FSCmdBlock diiServer_fileBlock;
extern FSClient diiServer_fileClient;
extern char * diiServer_patchFiles;
extern int diiServer_fileHandles[NUM_FILE_HANDLES];

extern char diiServer_stack[STACK_SIZE];
#endif // C_RETAINS_VARS_H_
