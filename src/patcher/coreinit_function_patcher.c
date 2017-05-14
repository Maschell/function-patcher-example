/****************************************************************************
 * Copyright (C) 2016 Maschell
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "common/retain_vars.h"
#include "coreinit_function_patcher.h"

#include "utils/logger.h"

DECL(u32,nn_nex_RC4Encryption_EncryptDecrypt,void * unkwn1/* probably a nn::nex::Buffer */, void * input_output, u32 length){
    if(encryptionDirection == 0){
        log_printf("out:");
        for(u32 i = 0;i<length;i++){
            log_printf("%02X",*(char*)(input_output+i));
        }
        log_printf("\n");
    }

    u32 result = real_nn_nex_RC4Encryption_EncryptDecrypt(unkwn1,input_output,length);

    if(encryptionDirection == 1){
        log_printf("in :");
        for(u32 i = 0;i<length;i++){
            log_printf("%02X",*(char*)(input_output+i));
        }
        log_printf("\n");
    }

    return result;
}


DECL(void *,nn_nex_RC4Encryption_Encrypt,void * a,void * b){
    encryptionDirection = 0;
    //Not working... data is slightly off
    //log_printf("nn_nex_RC4Encryption_Encrypt %08X %08X\n",a,b);
    /*u32* a_ = (u32*)b;
    u32* datapointer = a_[4];
    u32 length = a_[5];
    log_printf("out:");
    for(u32 i = 0;i<length;i++){
        log_printf("%02X",*(char*)(datapointer+i));
    }
    log_printf("\n");*/
    return real_nn_nex_RC4Encryption_Encrypt(a,b); //is calling MARIO_KART_8_HOOK
}

DECL(void * ,nn_nex_RC4Encryption_Decrypt,void * a,void * b){
    encryptionDirection = 1;
    void * res = real_nn_nex_RC4Encryption_Decrypt(a,b); //is calling MARIO_KART_8_HOOK
    //Not working... data is slightly off
    /*u32* a_ = (u32*)b;
    u32* datapointer = a_[4];
    u32 length = a_[5];
    log_printf("in :");
    for(u32 i = 0;i<length;i++){
        log_printf("%02X",*(char*)(datapointer+i));
    }
    log_printf("\n");*/
    return res;
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Encrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Decrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_EncryptDecrypt),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

