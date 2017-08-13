#ifndef _DIIBUGGER_DEBUGGER_H_
#define _DIIBUGGER_DEBUGGER_H_

#include "dynamic_libs/os_types.h"
#include "dynamic_libs/fs_functions.h"
#include "common/diibugger_defs.h"
#include "system/OSThread.h"

#ifdef __cplusplus
extern "C" {
#endif

bool HANDLE_CRASH(u32 type, void * handler);

void InitializeDebugger();

u8 ProgramHandler_Initialize(OSContext *context);

u8 DSIHandler_Debug(OSContext *context);

u8 ISIHandler_Debug(OSContext *context);

u8 ProgramHandler_Debug(OSContext *context);

void HandleProgram();

void ReportCrash(u32 msg);

void HandleDSI();

void HandleISI();

u8 DSIHandler_Fatal(OSContext *context);

u8 ISIHandler_Fatal(OSContext *context);

u8 ProgramHandler_Fatal(OSContext *context);

void FatalCrashHandler();

#ifdef __cplusplus
}
#endif

#endif /* _DIIBUGGER_DEBUGGER_H_ */
