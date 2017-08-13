#ifndef _DIIBUGGER_SERVER_H_
#define _DIIBUGGER_SERVER_H_
#include "dynamic_libs/os_types.h"
#include "common/diibugger_defs.h"
#include "system/OSThread.h"
#include "diibugger/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitDiibugger();
void RPCServer(CThread *thread, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* _DIIBUGGER_SERVER_H_ */
