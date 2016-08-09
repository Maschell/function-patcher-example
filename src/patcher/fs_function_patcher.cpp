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

#include "fs_function_patcher.h"

#include "utils/logger.h"

DECL(int, FSInit, void)
{
    log_print("FSInit called\n");
    return real_FSInit();
}

DECL(int, FSOpenDir, void *pClient, void* pCmd, const char *path, int *handle, int error) {
    log_print("FSOpenDir called\n");
    return real_FSOpenDir(pClient, pCmd, path, handle, error);
}

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSInit,      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDir,   LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

