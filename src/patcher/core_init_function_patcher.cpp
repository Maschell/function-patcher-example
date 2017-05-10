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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "core_init_function_patcher.hpp"

#include "utils/logger.h"

extern volatile u8 gEndThread;

DECL(void, __PPCExit, void){
    log_printf("PPCEXIT\n");
    gEndThread = 1;

    real___PPCExit();
}

hooks_magic_t method_hooks_core_init[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(__PPCExit,                           LIB_CORE_INIT,  STATIC_FUNCTION)
};

u32 method_hooks_size_core_init __attribute__((section(".data"))) = sizeof(method_hooks_core_init) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile u32 method_calls_core_init[sizeof(method_hooks_core_init) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

