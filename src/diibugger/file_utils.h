#ifndef _DIIBUGGER_FILE_UTILS_H_
#define _DIIBUGGER_FILE_UTILS_H_

#include "dynamic_libs/os_types.h"
#include "dynamic_libs/fs_functions.h"
#include "common/diibugger_defs.h"
#include "system/OSThread.h"

#ifdef __cplusplus
extern "C" {
#endif

int *GetServerHandle(int handle);
int *GetFreeFileHandle();
void SetServerFilePos(int handle, u32 pos);
int ReadServerFile(void *buffer, u32 size, u32 count, int handle);

bool IsServerFile(const char *path);
void OpenServerFile(const char *path, const char *mode, int *handle);
u32 GetPersistentId(u8 slot);

int OpenSaveFile(FSClient *client, u8 account, const char *path, const char *mode, int *handle);
#ifdef __cplusplus
}
#endif

#endif /* _DIIBUGGER_FILE_UTILS_H_ */
