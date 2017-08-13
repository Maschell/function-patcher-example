#include "coreinit_function_patcher.h"

#include "utils/logger.h"
#include "common/c_retain_vars.h"

DECL(void, _Exit, void){
    log_print("Application closed.\n");

    if(diiServer_threadPointer != 0){
        log_printf("Killing the RPC thread \n");
        diiServer_threadPointer->shutdownThread();
        log_printf("Killing the RPC thread done.\n");
        diiServer_threadPointer = 0;
    }

    real__Exit();
}

DECL(u32, OSIsDebuggerInitialized, void){
    return 1;
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(_Exit,                           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSIsDebuggerInitialized,         LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

