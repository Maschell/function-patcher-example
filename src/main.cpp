#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "common/common.h"
#include "common/c_retain_vars.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/nn_act_functions.h"
#include "dynamic_libs/nn_save_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "patcher/coreinit_function_patcher.h"
#include "patcher/fs_function_patcher.h"
#include "patcher/save_function_patcher.h"
#include "patcher/proc_ui_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "diibugger/server.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */
extern "C" int Menu_Main(void)
{
    if(gAppStatus == 2){
        return EXIT_RELAUNCH_ON_LOAD;
    }
    gAppStatus = 0;
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    InitSysFunctionPointers(); // For SYSLaunchMenu()
    InitSaveFunctionPointers();
    InitACTFunctionPointers();
    InitFSFunctionPointers();
    InitProcUIFunctionPointers();

    //For patching
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();

    SetupKernelCallback();

    log_init("192.168.0.181");

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        log_print("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    if(!isFirstBoot){
        log_print("Patching functions\n");
        ApplyPatches();
        InitDiibugger();
    }

    //!*******************************************************************
    //!                        Patching functions                        *
    //!*******************************************************************



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

/*
    Patching all the functions!!!
*/
void ApplyPatches(){
    PatchInvidualMethodHooks(method_hooks_coreinit,     method_hooks_size_coreinit,     method_calls_coreinit);
    PatchInvidualMethodHooks(method_hooks_fs,           method_hooks_size_fs,           method_calls_fs);
    PatchInvidualMethodHooks(method_hooks_save,         method_hooks_size_save,         method_calls_save);
    PatchInvidualMethodHooks(method_hooks_proc_ui,      method_hooks_size_proc_ui,      method_calls_proc_ui);
}

/*
    Restoring everything!!
*/

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_coreinit,  method_hooks_size_coreinit);
    RestoreInvidualInstructions(method_hooks_fs,        method_hooks_size_fs);
    RestoreInvidualInstructions(method_hooks_save,      method_hooks_size_save);
    RestoreInvidualInstructions(method_hooks_proc_ui,   method_hooks_size_proc_ui);
    KernelRestoreInstructions();
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
