#include <stdio.h>
#include <string.h>
#include "diibugger/utils.h"
#include "diibugger/debugger.h"
#include "common/c_retain_vars.h"

bool HANDLE_CRASH(u32 type, void * handler,OSContext * context){
    memcpy((char *)&diiServer_crashContext, (const char *)context, sizeof(OSContext));
    diiServer_crashType = type;
    context->srr0 = (u32)handler;
    return true;
}


void FatalCrashHandler() {
    char buffer[0x400];
    snprintf(buffer, 0x400,
        "An exception of type %i occurred:\n\n"
        "r0: %08X r1: %08X r2: %08X r3: %08X r4: %08X\n"
        "r5: %08X r6: %08X r7: %08X r8: %08X r9: %08X\n"
        "r10:%08X r11:%08X r12:%08X r13:%08X r14:%08X\n"
        "r15:%08X r16:%08X r17:%08X r18:%08X r19:%08X\n"
        "r20:%08X r21:%08X r22:%08X r23:%08X r24:%08X\n"
        "r25:%08X r26:%08X r27:%08X r28:%08X r29:%08X\n"
        "r30:%08X r31:%08X\n\n"
        "CR: %08X LR: %08X CTR:%08X XER:%08X\n"
        "EX0:%08X EX1:%08X SRR0:%08X SRR1:%08X\n",
        (unsigned int) diiServer_crashType,
        (unsigned int) diiServer_crashContext.gpr[0],
        (unsigned int) diiServer_crashContext.gpr[1],
        (unsigned int) diiServer_crashContext.gpr[2],
        (unsigned int) diiServer_crashContext.gpr[3],
        (unsigned int) diiServer_crashContext.gpr[4],
        (unsigned int) diiServer_crashContext.gpr[5],
        (unsigned int) diiServer_crashContext.gpr[6],
        (unsigned int) diiServer_crashContext.gpr[7],
        (unsigned int) diiServer_crashContext.gpr[8],
        (unsigned int) diiServer_crashContext.gpr[9],
        (unsigned int) diiServer_crashContext.gpr[10],
        (unsigned int) diiServer_crashContext.gpr[11],
        (unsigned int) diiServer_crashContext.gpr[12],
        (unsigned int) diiServer_crashContext.gpr[13],
        (unsigned int) diiServer_crashContext.gpr[14],
        (unsigned int) diiServer_crashContext.gpr[15],
        (unsigned int) diiServer_crashContext.gpr[16],
        (unsigned int) diiServer_crashContext.gpr[17],
        (unsigned int) diiServer_crashContext.gpr[18],
        (unsigned int) diiServer_crashContext.gpr[19],
        (unsigned int) diiServer_crashContext.gpr[20],
        (unsigned int) diiServer_crashContext.gpr[21],
        (unsigned int) diiServer_crashContext.gpr[22],
        (unsigned int) diiServer_crashContext.gpr[23],
        (unsigned int) diiServer_crashContext.gpr[24],
        (unsigned int) diiServer_crashContext.gpr[25],
        (unsigned int) diiServer_crashContext.gpr[26],
        (unsigned int) diiServer_crashContext.gpr[27],
        (unsigned int) diiServer_crashContext.gpr[28],
        (unsigned int) diiServer_crashContext.gpr[29],
        (unsigned int) diiServer_crashContext.gpr[30],
        (unsigned int) diiServer_crashContext.gpr[31],
        (unsigned int) diiServer_crashContext.cr,
        (unsigned int) diiServer_crashContext.lr,
        (unsigned int) diiServer_crashContext.ctr,
        (unsigned int) diiServer_crashContext.xer,
        (unsigned int) diiServer_crashContext.ex0,
        (unsigned int) diiServer_crashContext.ex1,
        (unsigned int) diiServer_crashContext.srr0,
        (unsigned int) diiServer_crashContext.srr1
    );

    OSFatal(buffer);
}

u8 DSIHandler_Fatal(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_DSI, (void*)FatalCrashHandler,context); }
u8 ISIHandler_Fatal(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_ISI, (void*)FatalCrashHandler,context); }
u8 ProgramHandler_Fatal(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_PROGRAM, (void*)FatalCrashHandler,context); }

void ReportCrash(u32 msg) {
    diiServer_crashState = CRASH_STATE_UNRECOVERABLE;

    OSMessage message;
    message.message = msg;
    message.data0 = (u32)&diiServer_crashContext;
    message.data1 = sizeof(diiServer_crashContext);
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    while (true) {
        OSSleepTicks(1000000);
    }
}

void HandleDSI() {
    ReportCrash(SERVER_MESSAGE_DSI);
}

void HandleISI() {
    ReportCrash(SERVER_MESSAGE_ISI);
}


void HandleProgram() {
    //Check if the exception was caused by a breakpoint
    if (!(diiServer_crashContext.srr1 & 0x20000)) {
        ReportCrash(SERVER_MESSAGE_PROGRAM);
    }

    //A breakpoint is done by replacing an instruction by a "trap" instruction
    //When execution is continued this instruction still has to be executed
    //So we have to put back the original instruction, execute it, and insert
    //the breakpoint again

    //We can't simply use the BE and SE bits in the MSR without kernel patches
    //However, since they're optional, they might not be implemented on the Wii U
    //Patching the kernel is not really worth the effort in this case, so I'm
    //simply placing a trap at the next instruction

    //Special case, the twu instruction at the start
    u32  entryPoint = 0x1005E040; //!TODO!!!
    log_printf("diiServer_crashContext.srr0 = %08X",diiServer_crashContext.srr0);
    if (diiServer_crashContext.srr0 == (u32)entryPoint + 0x48) {
        WriteCode(diiServer_crashContext.srr0, 0x60000000); //nop
    }

    if (diiServer_stepState == STEP_STATE_RUNNING || diiServer_stepState == STEP_STATE_STEPPING) {
        diiServer_crashState = CRASH_STATE_BREAKPOINT;

        OSMessage message;
        message.message = SERVER_MESSAGE_PROGRAM;
        message.data0 = (u32)&diiServer_crashContext;
        message.data1 = sizeof(diiServer_crashContext);
        OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);

        OSReceiveMessage(&diiServer_clientQueue, &message, OS_MESSAGE_BLOCK);

        if (diiServer_stepState == STEP_STATE_STEPPING) {
            RestoreStepInstructions();
        }

        breakpoint *bp = GetBreakPoint(diiServer_crashContext.srr0, 10);
        if (bp) {
            WriteCode(bp->address, bp->instruction);
        }

        //A conditional branch can end up at two places, depending on
        //wheter it's taken or not. To work around this, I'm using a
        //second, optional address. This is less work than writing code
        //that checks the condition registers.
        if ((u32)message.message == CLIENT_MESSAGE_STEP_OVER) {
            PredictStepAddresses(true);
        }
        else PredictStepAddresses(false);

        diiServer_breakpoints[STEP1].instruction = *(u32 *)(diiServer_breakpoints[STEP1].address);
        WriteCode(diiServer_breakpoints[STEP1].address, TRAP);
        if (diiServer_breakpoints[STEP2].address) {
            diiServer_breakpoints[STEP2].instruction = *(u32 *)(diiServer_breakpoints[STEP2].address);
            WriteCode(diiServer_breakpoints[STEP2].address, TRAP);
        }

        diiServer_stepSource = diiServer_crashContext.srr0;

        if ((u32)message.message == CLIENT_MESSAGE_CONTINUE) diiServer_stepState = STEP_STATE_CONTINUE;
        else diiServer_stepState = STEP_STATE_STEPPING;
    }
    else if (diiServer_stepState == STEP_STATE_CONTINUE) {
        RestoreStepInstructions();
        diiServer_stepState = STEP_STATE_RUNNING;
        diiServer_crashState = CRASH_STATE_NONE;
    }
    OSLoadContext(&diiServer_crashContext); //Resume execution
}

u8 DSIHandler_Debug(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_DSI, (void *) HandleDSI,context); }
u8 ISIHandler_Debug(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_ISI, (void *) HandleISI,context); }
u8 ProgramHandler_Debug(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_PROGRAM, (void *) HandleProgram,context); }



void InitializeDebugger() {
    if (!WaitForConnection()) {
        //If the game was started without debugger, use the OSFatal exception handler
        OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_PROGRAM, ProgramHandler_Fatal);
        //Advance the program counter to the next instruction, because it's still on the breakpoint instruction.
        diiServer_crashContext.srr0 += 4;
    }
    OSLoadContext(&diiServer_crashContext);
}

u8 ProgramHandler_Initialize(OSContext *context) { return HANDLE_CRASH(OS_EXCEPTION_PROGRAM, (void *) InitializeDebugger,context); }
