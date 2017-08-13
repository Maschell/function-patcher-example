#include <stdio.h>
#include <string.h>
#include "save_function_patcher.h"
#include "diibugger/server.h"
#include "diibugger/file_utils.h"

#include "common/diibugger_defs.h"
#include "common/c_retain_vars.h"
#include "dynamic_libs/os_types.h"
#include "dynamic_libs/fs_functions.h"
#include "utils/logger.h"

DECL(u32,SAVEOpenFile,FSClient *client, FSCmdBlock *block, u8 account, const char *path, const char *mode, int *handle, int errHandling) {
    if(gAppStatus == 2) return real_SAVEOpenFile(client,block,account,path,mode,handle,errHandling);
    int result = OpenSaveFile(client, account, path, mode, handle);
    if (result == -1) {
        return (u32)SAVEOpenFile + 0x2C;
    }
    return (u32)SAVEOpenFile + 0x1E8;
}

hooks_magic_t method_hooks_save[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(SAVEOpenFile,           LIB_SAVE,  STATIC_FUNCTION),
};

u32 method_hooks_size_save __attribute__((section(".data"))) = sizeof(method_hooks_save) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_save[sizeof(method_hooks_save) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

