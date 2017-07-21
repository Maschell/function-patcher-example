/****************************************************************************
 * Copyright (C) 2017 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "coreinit_function_patcher.h"
#include "utils/logger.h"
#include "utils/voice_swapper.hpp"
#include "common/c_retain_vars.h"
#include "system/memory.h"
#include "dynamic_libs/proc_ui_functions.h"

DECL(s32, AXSetVoiceDeviceMixOld, void *v, s32 device, u32 id, void *mix){
    if(gSwap) (device == 1) ? device = 0: device = 1;
    if(VOICE_SWAP_LOG == 1){log_printf("AXSetVoiceDeviceMixOld voice: %08X device: %d, mix: %08X\n",v,device,mix);}
    VoiceSwapper::setMix(v,device,mix);
    return real_AXSetVoiceDeviceMixOld(v,device,id,mix);
}

DECL(s32, AXSetVoiceDeviceMix, void *v, s32 device, u32 id, void *mix){
    if(gSwap) (device == 1) ? device = 0: device = 1;
    if(VOICE_SWAP_LOG == 1){log_printf("AXSetVoiceDeviceMix voice: %08X device: %d, mix: %08X\n",v,device,mix);}
    VoiceSwapper::setMix(v,device,mix);
    return real_AXSetVoiceDeviceMix(v,device,id,mix);
}

DECL(void *, AXAcquireVoiceExOld, u32 prio, void * callback, u32 arg){
    void * result = real_AXAcquireVoiceExOld(prio,callback,arg);
    if(VOICE_SWAP_LOG == 1){log_printf("AXAcquireVoiceExOld result: %08X \n",result);}
    VoiceSwapper::acquireVoice(result);
    return result;
}

DECL(void *, AXAcquireVoiceEx, u32 prio, void * callback, u32 arg){
    void * result = real_AXAcquireVoiceEx(prio,callback,arg);
    if(VOICE_SWAP_LOG == 1){log_printf("AXAcquireVoiceEx result: %08X \n",result);}
    VoiceSwapper::acquireVoice(result);
    return result;
}

DECL(void, AXFreeVoiceOld, void *v){
    if(VOICE_SWAP_LOG == 1){log_printf("AXFreeVoiceOld v: %08X \n",v);}
    VoiceSwapper::freeVoice(v);
    real_AXFreeVoiceOld(v);
}

DECL(void, AXFreeVoice, void *v){
    if(VOICE_SWAP_LOG == 1){log_printf("AXFreeVoice v: %08X \n",v);}
    VoiceSwapper::freeVoice(v);
    real_AXFreeVoice(v);
}

DECL(void, GX2CopyColorBufferToScanBuffer, GX2ColorBuffer *colorBuffer, s32 scan_target){
    if(gSwap) (scan_target == 1) ? scan_target = 4: scan_target = 1;
    real_GX2CopyColorBufferToScanBuffer(colorBuffer,scan_target);
}

/*
DECL(s32, AXSetDefaultMixerSelectOld, u32 s){
    s32 result = real_AXSetDefaultMixerSelectOld(s);
    return result;
}*/


void swapVoices(){
    VoiceSwapper::swapAll();
    for(int i = 0;i<VOICE_INFO_MAX;i++){
        if(gVoiceInfos[i].voice == NULL) continue;

        real_AXSetVoiceDeviceMix(gVoiceInfos[i].voice,0,0,gVoiceInfos[i].mixTV);
        real_AXSetVoiceDeviceMix(gVoiceInfos[i].voice,1,0,gVoiceInfos[i].mixDRC);
        real_AXSetVoiceDeviceMixOld(gVoiceInfos[i].voice,0,0,gVoiceInfos[i].mixTV);
        real_AXSetVoiceDeviceMixOld(gVoiceInfos[i].voice,1,0,gVoiceInfos[i].mixDRC);
    }
}

DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {
    int result = real_VPADRead(chan, buffer, buffer_size, error);

    if(result > 0 && ((buffer[0].btns_h & gButtonCombo) == gButtonCombo) && gCallbackCooldown == 0 ){
        gCallbackCooldown = 0x3C;
        gSwap = !gSwap;
        if(!gAppStatus){
            swapVoices();
        }
    }
    if(gCallbackCooldown > 0) gCallbackCooldown--;

    return result;
}

DECL(void, __PPCExit, void){
    real___PPCExit();
}

DECL(u32, ProcUIProcessMessages, u32 u){
    u32 res = real_ProcUIProcessMessages(u);
    if(res != gAppStatus){
        log_printf("App status changed from %d to %d \n",gAppStatus,res);
        gAppStatus = res;
    }

    return res;
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(     ProcUIProcessMessages,                          LIB_PROC_UI,    STATIC_FUNCTION),
    MAKE_MAGIC(     __PPCExit,                                      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(     GX2CopyColorBufferToScanBuffer,                 LIB_GX2,        STATIC_FUNCTION),
    MAKE_MAGIC(     VPADRead,                                       LIB_VPAD,       STATIC_FUNCTION),
    MAKE_MAGIC_NAME(AXAcquireVoiceExOld,    AXAcquireVoiceEx,       LIB_AX_OLD,     STATIC_FUNCTION),
    MAKE_MAGIC_NAME(AXFreeVoiceOld,         AXFreeVoice,            LIB_AX_OLD,     STATIC_FUNCTION),
    MAKE_MAGIC_NAME(AXSetVoiceDeviceMixOld, AXSetVoiceDeviceMix,    LIB_AX_OLD,     STATIC_FUNCTION),
    //MAKE_MAGIC_NAME(AXSetDefaultMixerSelectOld, AXSetDefaultMixerSelect,    LIB_AX_OLD,     STATIC_FUNCTION),
    MAKE_MAGIC_NAME(AXAcquireVoiceEx,       AXAcquireVoiceEx,       LIB_AX,         DYNAMIC_FUNCTION),
    MAKE_MAGIC_NAME(AXFreeVoice,            AXFreeVoice,            LIB_AX,         DYNAMIC_FUNCTION),
    MAKE_MAGIC_NAME(AXSetVoiceDeviceMix,    AXSetVoiceDeviceMix,    LIB_AX,         DYNAMIC_FUNCTION),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

