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
#ifndef _VOICE_INFO_H_
#define _VOICE_INFO_H_

#include "dynamic_libs/ax_functions.h"
#include "voice_swapper.hpp"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class VoiceInfo{

public:

    VoiceInfo(void * voice){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo] Constructor! Called with %08X\n",voice);}
        this->v = voice;
        memset(buffer, 0, sizeof(u32)*24);
        memset(mixTV, 0, sizeof(u32)*24);
        memset(mixDRC, 0, sizeof(u32)*24);
    }
    ~VoiceInfo(){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo(%08X)] Destructor!\n",v);}
    }

    void setTVMix(void* mix){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo(%08X)] setTVMix %08X!\n",v,mix);}
        setMix(0,mix);
    }

    void setDRCMix(void* mix){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo(%08X)] setDRCMix %08X!\n",v,mix);}
        setMix(1,mix);
    }

    void setMix(u32 device, void* mix){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo(%08X)] setMix device: %d mix: %08X!\n",v,device,mix);}
        if(mix == NULL) return;
        if(device == 0){
            memcpy(mixTV,mix,sizeof(u32)*24);
        }else if(device == 1){
            memcpy(mixDRC,mix,sizeof(u32)*24);
        }
    }

    void swapSounds(){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceInfo(%08X)] swapping sound!!\n",v);}
        memcpy(buffer,mixTV,sizeof(u32)*24);
        memcpy(mixTV,mixDRC,sizeof(u32)*24);
        memcpy(mixDRC,buffer,sizeof(u32)*24);
    }

    void * getVoice(){
        return v;
    }

    u32 * getTVMix(){
        return mixTV;
    }

    u32 * getDRCMix(){
        return mixDRC;
    }

    void* v = NULL;

    u32 buffer[24];
    u32 mixTV[24];
    u32 mixDRC[24];
};

#endif //_VOICE_INFO_H_
