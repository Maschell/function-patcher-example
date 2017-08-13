#include <stdio.h>
#include <string.h>
#include "diibugger/utils.h"
#include "../dynamic_libs/socket_functions.h"
#include "common/c_retain_vars.h"
#include "dynamic_libs/vpad_functions.h"

void CHECK_SOCKET(s32 result, const char * funcname){
    CHECK_ERROR_(result,funcname,socketlasterr());
}

void CHECK_ERROR(s32 result, const char * funcname){
    CHECK_ERROR_(result,funcname,result);
}

void CHECK_ERROR_(s32 result, const char * funcname, int printResult){
    if (result < 0) {
        char buffer[50] = {0};
        snprintf(buffer, 50, "%s failed: %i", funcname, printResult);
        OSFatal(buffer);
    }
}

breakpoint *GetBreakPoint(u32 addr, int end) {
    breakpoint *bplist = diiServer_breakpoints;
    for (int i = 0; i < end; i++) {
        if (bplist[i].address == addr) {
            return &bplist[i];
        }
    }
    return 0;
}

void RestoreStepInstructions() {
    //Write back the instructions that were replaced for the step
    WriteCode(diiServer_breakpoints[STEP1].address, diiServer_breakpoints[STEP1].instruction);
    diiServer_breakpoints[STEP1].address = 0;
    diiServer_breakpoints[STEP1].instruction = 0;
    if (diiServer_breakpoints[STEP2].address) {
        WriteCode(diiServer_breakpoints[STEP2].address, diiServer_breakpoints[STEP2].instruction);
        diiServer_breakpoints[STEP2].address = 0;
        diiServer_breakpoints[STEP2].instruction = 0;
    }

    breakpoint *bp = GetBreakPoint(diiServer_stepSource, 10);
    if (bp) {
        WriteCode(bp->address, TRAP);
    }
}

u32 GetInstruction(u32 address) {
    breakpoint *bp = GetBreakPoint(address, 12);
    if (bp) {
        return bp->instruction;
    }
    return *(u32 *)address;
}

void PredictStepAddresses(bool stepOver) {
    u32 currentAddr = diiServer_crashContext.srr0;
    u32 instruction = GetInstruction(currentAddr);

    breakpoint *step1 = &diiServer_breakpoints[STEP1];
    breakpoint *step2 = &diiServer_breakpoints[STEP2];
    step1->address = currentAddr + 4;
    step2->address = 0;

    u8 opcode = instruction >> 26;
    if (opcode == 19) {
        u16 XO = (instruction >> 1) & 0x3FF;
        bool LK = instruction & 1;
        if (!LK || !stepOver) {
            if (XO ==  16) step2->address = diiServer_crashContext.lr; //bclr
            if (XO == 528) step2->address = diiServer_crashContext.ctr; //bcctr
        }
    }

    else if (opcode == 18) { //b
        bool AA = instruction & 2;
        bool LK = instruction & 1;
        u32 LI = instruction & 0x3FFFFFC;
        if (!LK || !stepOver) {
            if (AA) step1->address = LI;
            else {
                if (LI & 0x2000000) LI -= 0x4000000;
                step1->address = currentAddr + LI;
            }
        }
    }

    else if (opcode == 16) { //bc
        bool AA = instruction & 2;
        bool LK = instruction & 1;
        u32 BD = instruction & 0xFFFC;
        if (!LK || !stepOver) {
            if (AA) step2->address = BD;
            else {
                if (BD & 0x8000) BD -= 0x10000;
                step2->address = currentAddr + BD;
            }
        }
    }
}

void WriteCode(u32 address, u32 instr) {
    u32 *ptr = (u32 *)(address + 0xA0000000);

    *ptr = instr;//TODO: kernel copy data???
    //SC0x25_KernelCopyData((u32)OSEffectiveToPhysical(ptr), (u32)OSEffectiveToPhysical(&instr), 4);

    DCFlushRange(ptr, 4);
    ICInvalidateRange(ptr, 4);
}



bool isValidStackPtr(u32 sp) {
    return sp >= 0x10000000 && sp < 0x20000000;
}

u32 PushThread(char *buffer, u32 offset, OSThread *thread) {
    *(u32 *)(buffer + offset) = OSGetThreadAffinity(thread);
    *(u32 *)(buffer + offset + 4) = OSGetThreadPriority(thread);
    *(u32 *)(buffer + offset + 8) = (u32)thread->stackBase;
    *(u32 *)(buffer + offset + 12) = (u32)thread->stackEnd;
    *(u32 *)(buffer + offset + 16) = (u32)thread->entryPoint;

    const char *threadName = OSGetThreadName(thread);
    if (threadName) {
        u32 namelen = strlen(threadName);
        *(u32 *)(buffer + offset + 20) = namelen;
        memcpy(buffer + offset + 24, threadName, namelen);
        return 24 + namelen;
    }

    *(u32 *)(buffer + offset + 20) = 0;
    return 24;
}

breakpoint *GetFreeBreakPoint() {
    breakpoint *bplist = diiServer_breakpoints;
    for (int i = 0; i < 10; i++) {
        if (bplist[i].address == 0) {
            return &bplist[i];
        }
    }
    return 0;
}

breakpoint *GetBreakPointRange(u32 addr, u32 num, breakpoint *prev) {
    breakpoint *bplist = diiServer_breakpoints;

    int start = 0;
    if (prev) {
        start = (prev - bplist) + 1;
    }

    for (int i = start; i < 12; i++) {
        if (bplist[i].address >= addr && bplist[i].address < addr + num) {
            return &bplist[i];
        }
    }
    return 0;
}

void WriteScreen(const char *msg) {
    for (int buffer = 0; buffer < 2; buffer++) {
        OSScreenClearBufferEx(buffer, 0);
        OSScreenPutFontEx(buffer, 0, 0, msg);
        OSScreenFlipBuffersEx(buffer);
    }
}

bool WaitForConnection() {
    OSScreenInit();
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)(0xF4000000 + OSScreenGetBufferSizeEx(0)));
    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);
    WriteScreen("Waiting for debugger connection.\n"
                "Press the home button to continue without debugger.\n"
                "You can still connect while the game is running.");

    s32 error;
    VPADData status;
    VPADRead(0, &status, 1, &error);
    while (!(status.btns_h & VPAD_BUTTON_HOME)) {
        if (diiServer_connection) {
            //Another WriteScreen to update the displayed message here
            //somehow causes problems when the game tries to use GX2
            return true;
        }
        VPADRead(0, &status, 1, &error);
    }
    return false;
}
