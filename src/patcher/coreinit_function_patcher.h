#ifndef _CORE_INIT_FUNCTION_PATCHER_H
#define _CORE_INIT_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t method_hooks_coreinit[];
extern u32 method_hooks_size_coreinit;
extern volatile unsigned int method_calls_coreinit[];

#ifdef __cplusplus
}
#endif

#endif /* _CORE_INIT_FUNCTION_PATCHER_H */
