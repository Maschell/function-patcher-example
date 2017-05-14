#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "common/common.h"
#include "common/os_defs.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "patcher/coreinit_function_patcher.h"
#include "patcher/fs_function_patcher.h"
#include "patcher/pad_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "utils/logger.h"
#include "common/retain_vars.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

EXPORT_DECL(void, start, void );
EXPORT_DECL(void, _start, void );
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

    //For patching
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();

    SetupKernelCallback();

    log_init("192.168.0.181");

    u32 * main_entry_addr = (u32*)*((u32*)OS_SPECIFICS->addr_OSTitle_main_entry);

    u32 startAddressInRPX = 0x026774B8;             //Address of MK8 start function in Turbo.rpx v64
    u32 encryptAddressInRPX = 0x0291A44C;           //Address of MK8 nn::nex::RC4Encryption::Encrypt in Turbo.rpx v64
    u32 decryptAddressInRPX = 0x0291A4DC;           //Address of MK8 nn::nex::RC4Encryption::Decrypt in Turbo.rpx v64
    u32 decryptencryptAddressInRPX = 0x0291A2B8;    //Address of MK8 nn::nex::RC4Encryption::EncryptDecrypt in Turbo.rpx v64
    u32 * encrypt_ptr  = (u32*)((u32)main_entry_addr + (encryptAddressInRPX-startAddressInRPX));
    u32 * decrypt_ptr  = (u32*)((u32)main_entry_addr + (decryptAddressInRPX-startAddressInRPX));
    u32 * decryptencrypt_ptr  = (u32*)((u32)main_entry_addr + (decryptencryptAddressInRPX-startAddressInRPX));

    if(*decryptencrypt_ptr == 0x9421FFE0){ //Check if value is as expected => MK8 EUR, v64 //TODO: proper check.
        //for(int i=0;i<10;i++){
        //    log_printf("%08X \n",decryptencrypt_ptr[i]);
        //}
        log_printf("Setting method_hooks_coreinit[0].realAddr to %08X\n",(u32)encrypt_ptr);
        log_printf("Setting method_hooks_coreinit[1].realAddr to %08X\n",(u32)decrypt_ptr);
        log_printf("Setting method_hooks_coreinit[2].realAddr to %08X\n",(u32)decryptencrypt_ptr);
        method_hooks_coreinit[0].realAddr = (u32)encrypt_ptr;
        method_hooks_coreinit[1].realAddr = (u32)decrypt_ptr;
        method_hooks_coreinit[2].realAddr = (u32)decryptencrypt_ptr;
        log_print("Patching functions\n");
        ApplyPatches();
    }


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
    //PatchInvidualMethodHooks(method_hooks_fs,           method_hooks_size_fs,           method_calls_fs);
    //PatchInvidualMethodHooks(method_hooks_pad,          method_hooks_size_pad,          method_calls_pad);
}

/*
    Restoring everything!!
*/

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_coreinit,  method_hooks_size_coreinit);
    //RestoreInvidualInstructions(method_hooks_fs,        method_hooks_size_fs);
    //RestoreInvidualInstructions(method_hooks_pad,       method_hooks_size_pad);
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
