#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <malloc.h>
#include "main.h"
#include "common/common.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/gx2_types.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/proc_ui_functions.h"
#include "patcher/coreinit_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "common/c_retain_vars.h"
#include "system/memory.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */
extern "C" int Menu_Main(void)
{
    if(gAppStatus == 2){
        log_printf("No, we don't want to patch stuff again.");
        return EXIT_RELAUNCH_ON_LOAD;
    }
    gAppStatus = 0;
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    log_init("192.168.0.181");

    InitSysFunctionPointers(); // For SYSLaunchMenu()
    InitProcUIFunctionPointers();

    //For patching
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();
    InitAXFunctionPointers();
    InitGX2FunctionPointers();

    SetupKernelCallback();

    memset(gVoiceInfos,0,sizeof(gVoiceInfos));

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        log_print("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    //!*******************************************************************
    //!                        Patching functions                        *
    //!*******************************************************************
    log_print("Patching functions\n");
    ApplyPatches();

    if(!isInMiiMakerHBL()){ //Starting the application
        return EXIT_RELAUNCH_ON_LOAD;
    }

    if(isFirstBoot){ // First boot back to SysMenu

        u32 res = SplashScreen(10,2);
        gButtonCombo = res;
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
}

/*
    Restoring everything!!
*/

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_coreinit,  method_hooks_size_coreinit);
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

#define FPS 60
u32 SplashScreen(s32 time,s32 combotime){
    u32 result = VPAD_BUTTON_TV;
    // Prepare screen
    s32 screen_buf0_size = 0;

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)(0xF4000000 + screen_buf0_size));

    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);

    // Clear screens
    OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);

    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    std::vector<std::string> strings;
    strings.push_back("SwipSwapMe 0.2 - by Maschell.");
    strings.push_back("");
    strings.push_back("");
    strings.push_back("Press the combo you want to use for swapping now for 2 seconds.");
    strings.push_back("Pressing the TV button will return directly.");
    strings.push_back("");
    strings.push_back("Otherwise the default combo (TV button) will be used in 10 seconds.");
    u8 pos = 0;
    for (std::vector<std::string>::iterator it = strings.begin() ; it != strings.end(); ++it){
        OSScreenPutFontEx(0, 0, pos, (*it).c_str());
        OSScreenPutFontEx(1, 0, pos, (*it).c_str());
        pos++;
    }

    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);

    s32 tickswait = time * FPS * 16;

    s32 sleepingtime = 16;
    s32 times = tickswait/16;
    s32 i=0;

    VPADData vpad_data;
    s32 error;
    u32 last = 0xFFFFFFFF;
    s32 timer = 0;
    while(i<times){
        VPADRead(0, &vpad_data, 1, &error);
        if(vpad_data.btns_h == VPAD_BUTTON_TV) break;
        if(last == vpad_data.btns_h && last != 0){
            timer++;
        }else{
            last = vpad_data.btns_h;
            timer = 0;
        }
        if(timer >= combotime*FPS){
            result = vpad_data.btns_h;
            break;
        }
        i++;
        os_usleep(sleepingtime*1000);
    }
    return result;
}
