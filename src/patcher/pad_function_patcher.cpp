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

#include "pad_function_patcher.h"
#include "utils/logger.h"

DECL(int, VPADRead, int chan, VPADData *buffer, u32 buffer_size, s32 *error) {
    int result = real_VPADRead(chan, buffer, buffer_size, error);
     if(buffer->btns_r&VPAD_BUTTON_STICK_R) {
            int mode;
            VPADGetLcdMode(0, &mode);       // Get current display mode
            if(mode != 1) {
                VPADSetLcdMode(0, 1);       // Turn it off
            }
            else {
                VPADSetLcdMode(0, 0xFF);    // Turn it on
            }
    }
    return result;
}

hooks_magic_t method_hooks_pad[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(VPADRead, LIB_VPAD,          STATIC_FUNCTION),
};

u32 method_hooks_size_pad __attribute__((section(".data"))) = sizeof(method_hooks_pad) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_pad[sizeof(method_hooks_pad) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

