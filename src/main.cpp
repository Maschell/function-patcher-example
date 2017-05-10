#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "common/common.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "kernel/kernel_functions.h"
#include "patcher/core_init_function_patcher.hpp"
#include "utils/function_patcher.h"
#include "utils/logger.h"
#include "system/CThread.h"

volatile u8 gEndThread __attribute__((section(".data"))) = 0;
volatile u8 isFirstBoot __attribute__((section(".data"))) = 1;
//Forward declaration
void mainThread(CThread *thread, void *arg);

/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    InitSysFunctionPointers(); // For SYSLaunchMenu()

    gEndThread = 0;

    SetupKernelCallback();

    log_init("192.168.0.181");

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        log_print("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    ApplyPatches();

    //Start Thread!
    CThread * pThread = CThread::create(mainThread, NULL, CThread::eAttributeAffCore2 | CThread::eAttributePinnedAff, 15);
    pThread->resumeThread();

    if(!isInMiiMakerHBL()){ //Starting the application
        return EXIT_RELAUNCH_ON_LOAD;
    }

    if(isFirstBoot){ // First boot back to SysMenu
        isFirstBoot = 0;
        SYSLaunchMenu();
        return EXIT_RELAUNCH_ON_LOAD;
    }

    deInit();
    return EXIT_SUCCESS;
}


void mainThread(CThread *thread, void *arg){
    while(!gEndThread){
        log_printf("Thread running..\n");
        os_usleep(1000*1000); //Wait for 1 second.
    }
    log_printf("Thread ended\n");
}

void ApplyPatches(){
    PatchInvidualMethodHooks(   method_hooks_core_init,    method_hooks_size_core_init,   method_calls_core_init);
}

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_core_init,    method_hooks_size_core_init);
}


void deInit(){
    RestorePatches();
    log_deinit();
}

s32 isInMiiMakerHBL(){
    if (OSGetTitleID != 0 && (
            OSGetTitleID() == 0x000500101004A200 || // mii maker eur
            OSGetTitleID() == 0x000500101004A100 || // mii maker usa
            OSGetTitleID() == 0x000500101004A000 ||// mii maker jpn
            OSGetTitleID() == 0x0005000013374842))
        {
            return 1;
    }
    return 0;
}
