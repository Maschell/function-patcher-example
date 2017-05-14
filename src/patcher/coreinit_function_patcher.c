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

/**
    http://stackoverflow.com/a/17147874
**/
void bin_to_strhex(unsigned char *bin, unsigned int binsz, char **result){
  char          hex_str[]= "0123456789ABCDEF";
  unsigned int  i;

  *result = (char *)malloc(binsz * 2 + 1);
  if(*result == NULL) return;
  (*result)[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++)
    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }
}


DECL(u32,nn_nex_RC4Encryption_EncryptDecrypt,void * unkwn1/* probably a nn::nex::Buffer */, void * input_output, u32 length){
    char *in_result;
    bin_to_strhex((unsigned char *)input_output, length, &in_result);
    if(in_result != NULL){
        if(encryptionDirection == 0){
            log_printf("[RC4] decrypted input : %s\n",in_result);
        }else{
            log_printf("[RC4] encrypted output: %s\n",in_result);
        }
    }
    free(in_result);

    u32 result = real_nn_nex_RC4Encryption_EncryptDecrypt(unkwn1,input_output,length);

    char *out_result;
    bin_to_strhex((unsigned char *)input_output, length, &out_result);
    if(out_result != NULL){
        if(encryptionDirection == 1){
            log_printf("[RC4] decrypted output: %s\n",out_result);
        }else{
            log_printf("[RC4] encrytped input : %s\n",out_result);
        }
    }
    free(out_result);


    return result;
}


DECL(void *,nn_nex_RC4Encryption_Encrypt,void * a,void * b){
    encryptionDirection = 0; //Setting the encryption direction and use the EncryptDecrypt Hook
    //Data should be ((u32*)b)[4]  length in ((u32*)b)[5] Not working... data is slightly off.
    return real_nn_nex_RC4Encryption_Encrypt(a,b); //is calling nn_nex_RC4Encryption_EncryptDecrypt
}

DECL(void * ,nn_nex_RC4Encryption_Decrypt,void * a,void * b){
    encryptionDirection = 1; //Setting the encryption direction and use the EncryptDecrypt Hook
    void * res = real_nn_nex_RC4Encryption_Decrypt(a,b); //is calling nn_nex_RC4Encryption_EncryptDecrypt
    //Data should be ((u32*)b)[4]  length in ((u32*)b)[5] Not working... data is slightly off.
    return res;
}

 //Not logging
DECL(void * ,nn_nex_EncryptionAlgorithm_Encrypt,void * a,void * b){
    log_printf("nn_nex_EncryptionAlgorithm_Encrypt\n");
    return real_nn_nex_EncryptionAlgorithm_Encrypt(a,b);
}

//Not logging
DECL(void * ,nn_nex_EncryptionAlgorithm_Decrypt,void * a,void * b){
    log_printf("nn_nex_EncryptionAlgorithm_Decrypt\n");
    return real_nn_nex_EncryptionAlgorithm_Decrypt(a,b);
}

/* Mario Kart 8 doesn't use this.
DECL(s32, send, s32 s, const void *buffer, s32 size, s32 flags){
    char *result;
    bin_to_strhex((unsigned char *)buffer, size, &result);
    if(result != NULL){
        log_printf("[send]     socket: %08X, flags %08X size: %08X data: %s\n",s,flags,size,result);
    }else{
        log_printf("[send]     socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,size);
    }
    return real_send(s,buffer,size,flags);

}

DECL(s32, recv, s32 s, void *buffer, s32 size, s32 flags){
    s32 result_size = real_recv(s,buffer,size,flags);
    char *result;
    bin_to_strhex((unsigned char *)buffer, result_size, &result);
    if(result != NULL){
        log_printf("[recv]     socket: %08X, flags %08X size: %08X data: %s\n",s,flags,result_size,result);
    }else{
        log_printf("[recv]     socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,result_size);
    }
    return result_size;
}*/


DECL(s32, recvfrom,s32 s, void *buffer, s32 size, s32 flags,struct sockaddr *src_addr, s32 *addrlen){
    s32 result_size = real_recvfrom(s,buffer,size,flags,src_addr,addrlen);
    char *result;
    bin_to_strhex((unsigned char *)buffer, result_size, &result);
    char *ip = NULL;
    if(*addrlen > 0 && src_addr != NULL ){
        ip = inet_ntoa(((struct sockaddr_in *)src_addr)->sin_addr);
    }
    if(result != NULL){
        if(ip == NULL){
            log_printf("[recvfrom] socket: %08X, flags %08X size: %08X data: %s\n",s,flags,result_size,result);
        }else{
            log_printf("[recvfrom] socket: %08X, addr: %s, flags %08X size: %08X data: %s\n",s,ip,flags,result_size,result);
        }
    }else{
        log_printf("[recvfrom] socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,result_size,result);
    }
    return result_size;
}

DECL(s32, sendto, s32 s, const void *buffer, s32 size, s32 flags, const struct sockaddr *dest, s32 dest_len){
    char *result;
    bin_to_strhex((unsigned char *)buffer, size, &result);
    if(result != NULL){
        char *ip = NULL;
        if(dest_len > 0 && dest != NULL){
            ip = inet_ntoa(((struct sockaddr_in *)dest)->sin_addr);
        }
        if(ip == NULL){
            log_printf("[sendto]   socket: %08X, flags %08X size: %08X data: %s\n",s,flags,size,result);
        }else{
            log_printf("[sendto]   socket: %08X, addr: %s, flags %08X size: %08X data: %s\n",s,ip,flags,size,result);
        }
    }else{
        log_printf("[sendto]   socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,size,result);
    }
    return real_sendto(s,buffer,size,flags,dest,dest_len);
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Encrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Decrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_EncryptDecrypt),
    MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Encrypt),
    MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Decrypt),
    //MAKE_MAGIC(send, LIB_NSYSNET, STATIC_FUNCTION),
    //MAKE_MAGIC(recv, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(recvfrom, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(sendto, LIB_NSYSNET, STATIC_FUNCTION),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
