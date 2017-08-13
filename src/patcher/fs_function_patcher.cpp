
#include <stdio.h>
#include <string.h>
#include "fs_function_patcher.h"
#include "diibugger/server.h"
#include "diibugger/file_utils.h"

#include "common/c_retain_vars.h"
#include "common/diibugger_defs.h"
#include "dynamic_libs/os_types.h"
#include "dynamic_libs/fs_functions.h"
#include "utils/logger.h"

//TODO: PROCUI check?


DECL(int, FSGetStat, void *client, void *block, const char *path, FSStat *stat, int errHandling){
    log_printf("FSGetStat \n");
    if ((client == &diiServer_fileClient) || !IsServerFile(path) || gAppStatus == 2) return real_FSGetStat(client,block,path,stat,errHandling);

    OSMessage message;
    message.message = SERVER_MESSAGE_GET_STAT;
    message.data0 = (u32)path;
    message.data1 = strlen(path);
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    memset((char *)stat, 0, sizeof(FSStat));
    stat->size = message.data0;
    return 0;
}

DECL(int, FSOpenFile,void *client, void *block, const char *path, const char *mode, int *handle, int errHandling) {
    log_printf("FSOpenFile \n");
    if (client == &diiServer_fileClient || gAppStatus == 2) return real_FSOpenFile(client,block,path,mode,handle,errHandling);

    if (path[0] != '/') {
        char workPath[640] = {0};
        FSGetCwd(client, block, workPath, 640, -1);

        u32 length = strlen(workPath);
        if (workPath[length - 1] == '/') {
            workPath[length - 1] = '\x00';
        }

        char newPath[640] = {0};
        snprintf(newPath, 640, "%s/%s", workPath, path);
        path = newPath;
    }

    if (!(IsServerFile(path) || mode[0] == 'w')) return real_FSOpenFile(client,block,path,mode,handle,errHandling);
    OpenServerFile(path, mode, handle);
    return 0;
}

DECL(int, FSReadFile,void *client, void *block, void *buffer, u32 size, u32 count, int handle, u32 flags, int errHandling) {
    log_printf("FSReadFile \n");
    if (client == &diiServer_fileClient || !GetServerHandle(handle) || gAppStatus == 2) return real_FSReadFile(client,block,buffer,size,count,handle,flags,errHandling);
    return ReadServerFile(buffer, size, count, handle);
}

DECL(int, FSReadFileWithPos,FSClient *client, FSCmdBlock *block, void *buffer, u32 size, u32 count, u32 pos, int handle, u32 flags, int errHandling) {
    log_printf("FSReadFileWithPos \n");
    if ((client == &diiServer_fileClient) || !GetServerHandle(handle) || gAppStatus == 2) return real_FSReadFileWithPos(client,block,buffer,size,count,pos,handle,flags,errHandling);
    SetServerFilePos(handle, pos);
    return ReadServerFile(buffer, size, count, handle);
}

DECL(int, FSWriteFile,FSClient *client, FSCmdBlock *block, void *buffer, u32 size, u32 count, int handle, u32 flags, int errHandling) {
    log_printf("FSWriteFile \n");
    if ((client == &diiServer_fileClient) || !GetServerHandle(handle) || gAppStatus == 2) return real_FSWriteFile(client,block,buffer,size,count,handle,flags,errHandling);

    u32 writeArgs[] = {(u32)buffer, size, count, (u32)handle};
    OSMessage message;
    message.message = SERVER_MESSAGE_WRITE_FILE;
    message.data0 = (u32)writeArgs;
    message.data1 = 0x10;
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    return message.data0;
}

DECL(int, FSCloseFile,FSClient *client, FSCmdBlock *block, int handle, int errHandling) {
    log_printf("FSCloseFile \n");
    if (client == &diiServer_fileClient || gAppStatus == 2) return real_FSCloseFile(client,block,handle,errHandling);
    int *serverHandle = GetServerHandle(handle);
    if (!serverHandle) return real_FSCloseFile(client,block,handle,errHandling);

    OSMessage message;
    message.message = SERVER_MESSAGE_CLOSE_FILE;
    message.data0 = 0;
    message.data1 = 0;
    message.data2 = handle;
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    *serverHandle = 0;
    return 0;
}

DECL(int, FSSetPosFile,FSClient *client, FSCmdBlock *block, int handle, u32 pos, int errHandling) {
    log_printf("FSSetPosFile \n");
    if ((client == &diiServer_fileClient) || !GetServerHandle(handle) || gAppStatus == 2) return real_FSSetPosFile(client,block,handle,pos,errHandling);
    SetServerFilePos(handle, pos);
    return 0;
}

DECL(int, FSGetStatFile,FSClient *client, FSCmdBlock *block, int handle, FSStat *stat, int errHandling) {
    log_printf("FSGetStatFile \n");
    if ((client == &diiServer_fileClient) || !GetServerHandle(handle) || gAppStatus == 2) return real_FSGetStatFile(client,block,handle,stat,errHandling);

    OSMessage message;
    message.message = SERVER_MESSAGE_GET_STAT_FILE;
    message.data0 = 0;
    message.data1 = 0;
    message.data2 = handle;
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    stat->size = message.data0;
    return 0;
}


hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSGetStat,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFile ,         LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFile,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSCloseFile,         LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFile,        LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFile,       LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSWriteFile,         LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPos,   LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

