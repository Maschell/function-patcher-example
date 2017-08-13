#ifndef _DIIBUGGER_UTILS_H_
#define _DIIBUGGER_UTILS_H_

#include "dynamic_libs/os_types.h"
#include "common/diibugger_defs.h"
#include "system/OSThread.h"

#ifdef __cplusplus
extern "C" {
#endif

void CHECK_SOCKET(s32 result, const char * funcname);

void CHECK_ERROR(s32 result, const char * funcname);

void CHECK_ERROR_(s32 result, const char * funcname, int printResult);

breakpoint *GetBreakPoint(u32 addr, int end);

void WriteCode(u32 address, u32 instr);

void PredictStepAddresses(bool stepOver);

void RestoreStepInstructions();

u32 GetInstruction(u32 address);

breakpoint *GetBreakPointRange(u32 addr, u32 num, breakpoint *prev);

breakpoint *GetFreeBreakPoint();

u32 PushThread(char *buffer, u32 offset, OSThread *thread);

bool isValidStackPtr(u32 sp);

void WriteScreen(const char *msg);

bool WaitForConnection();

#ifdef __cplusplus
}
#endif

#endif /* _DIIBUGGER_UTILS_H_ */
