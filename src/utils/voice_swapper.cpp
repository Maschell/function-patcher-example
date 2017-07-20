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
#include "voice_swapper.hpp"

VoiceSwapper * VoiceSwapper::instance  __attribute__((section(".data"))) = NULL;

void VoiceSwapper::_acquireVoice(void * voice){
    if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] acquireVoice %08X\n",voice);}
    voices[voice] = new VoiceInfo(voice);
}

void VoiceSwapper::_freeVoice(void * voice){
    VoiceInfo* data = voices[voice];
    if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Deleting from list: VoiceInfo for voice %08X\n",voice);}
    if(data != NULL){
        if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Freeing VoiceInfo for voice %08X\n",voice);}
        free(data);
    }
    voices.erase(voice);
}

void VoiceSwapper::_setMix(void * voice,u32 device, void* mix){
    VoiceInfo* data = voices[voice];
    if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] setMix voice %08X device %d mix %08X!\n",voice,device,mix);}
    if(data != NULL) data->setMix(device,mix);
}

void VoiceSwapper::_swapAll(){
    for (std::map<void*,VoiceInfo*>::iterator it=voices.begin(); it!=voices.end(); ++it){
        VoiceInfo* info = it->second;
        if(info != NULL){
            if(VOICE_SWAP_LOG == 1){log_printf("[VoiceSwapper] Swapping voice %08X (%08X)!\n",it->first,it->second);}
            info->swapSounds();
        }
    }
}

std::vector<VoiceInfo*> VoiceSwapper::_getAllVoiceInfos(){
     std::vector<VoiceInfo*> info;
     for (std::map<void*,VoiceInfo*>::iterator it=voices.begin(); it!=voices.end(); ++it){
        info.push_back(it->second);
     }
     return info;
}
