#ifndef _SAVE_FUNCTION_PATCHER_H
#define _SAVE_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

extern hooks_magic_t method_hooks_save[];
extern u32 method_hooks_size_save;
extern volatile unsigned int method_calls_save[];

#ifdef __cplusplus
}
#endif

#endif /* _SAVE_FUNCTION_PATCHER_H */
