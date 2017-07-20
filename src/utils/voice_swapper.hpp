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
#ifndef _VOICE_SWAPPER_H_
#define _VOICE_SWAPPER_H_
#define VOICE_SWAP_LOG 2
#include "voice_info.hpp"
#include "utils/logger.h"
#include "common/c_retain_vars.h"
#include "dynamic_libs/os_functions.h"
#include <map>
#include <vector>

class VoiceSwapper{

public:
     static VoiceSwapper * getInstance() {
        if(!instance)
            instance = new VoiceSwapper();
        return instance;
    }

    static void destroyInstance() {
        if(instance){
            delete instance;
            instance = NULL;
        }
    }

    static void acquireVoice(void * voice){
        VoiceSwapper * instance = VoiceSwapper::getInstance();
        if(instance != NULL){
            instance->_acquireVoice(voice);
        }
    }

    static void freeVoice(void * voice){
        VoiceSwapper * instance = VoiceSwapper::getInstance();
        if(instance != NULL){
            instance->_freeVoice(voice);
        }
    }

    static void setMix(void * voice,u32 device, void* mix){
        VoiceSwapper * instance = VoiceSwapper::getInstance();
        if(instance != NULL){
            instance->_setMix(voice,device,mix);
        }
    }

    static void swapAll(){
        VoiceSwapper * instance = VoiceSwapper::getInstance();
        if(instance != NULL){
            instance->swapAll();
        }
    }

    static std::vector<VoiceInfo*> getAllVoiceInfos(){
        VoiceSwapper * instance = VoiceSwapper::getInstance();
        if(instance != NULL){
            return instance->_getAllVoiceInfos();
        }
        std::vector<VoiceInfo*> emptyList;
        return emptyList;
    }


private:
    VoiceSwapper(){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Creating the VoiceSwapper Instance!\n");}
    }
    ~VoiceSwapper(){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Deleting the VoiceSwapper Instance!\n");}
        for (std::map<void*,VoiceInfo*>::iterator it=voices.begin(); it!=voices.end(); ++it){
            VoiceInfo* info = it->second;
            if(info != NULL){
                if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Deleting voice %08X (%08X)!\n",it->first,it->second);}
                free(info);
            }
        }
    }

    void _acquireVoice(void * voice);
    void _freeVoice(void * voice);
    void _setMix(void * voice,u32 device, void* mix);
    void _swapAll();
    std::vector<VoiceInfo*> _getAllVoiceInfos();

    static VoiceSwapper * instance __attribute__((section(".data")));
    std::map<void*,VoiceInfo*> voices;

};

#endif //_VOICE_SWAPPER_H_
