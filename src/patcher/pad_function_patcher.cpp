/****************************************************************************
 * Copyright (C) 2016,2017 Maschell
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

#include <utils/logger.h>
#include "pad_function_patcher.h"

DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {
    int result = real_VPADRead(chan, buffer, buffer_size, error);
    log_print("VPADRead\n");
    return result;
}

DECL(s32, KPADRead, s32 chan, void * data, u32 size){
    s32 result = real_KPADRead(chan, data, size);
    log_print("KPADRead\n");
    return result;
}

DECL(void, WPADRead, s32 chan, void * data){
    real_WPADRead(chan, data);
    log_print("WPADRead\n");
}

hooks_magic_t method_hooks_pad[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(VPADRead, LIB_VPAD,          STATIC_FUNCTION),
    MAKE_MAGIC(KPADRead, LIB_PADSCORE,      DYNAMIC_FUNCTION),
    MAKE_MAGIC(WPADRead, LIB_PADSCORE,      DYNAMIC_FUNCTION),
};

u32 method_hooks_size_pad __attribute__((section(".data"))) = sizeof(method_hooks_pad) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile u32 method_calls_pad[sizeof(method_hooks_pad) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

