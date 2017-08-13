#include <stdio.h>
#include <string.h>
#include "diibugger/file_utils.h"
#include "common/c_retain_vars.h"
#include "dynamic_libs/nn_act_functions.h"

int *GetServerHandle(int handle) {
    for (int i = 0; i < NUM_FILE_HANDLES; i++) {
        if (diiServer_fileHandles[i] == handle) {
            return &diiServer_fileHandles[i];
        }
    }
    return 0;
}

int *GetFreeFileHandle() {
    int *handle = GetServerHandle(0);
    if (!handle) {
        OSFatal("All file handles occupied\n");
    }
    return handle;
}

void SetServerFilePos(int handle, u32 pos) {
    u32 args[] = {(u32)handle, pos};
    OSMessage message;
    message.message = SERVER_MESSAGE_SET_POS_FILE;
    message.data0 = (u32)args;
    message.data1 = 8;
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
}

int ReadServerFile(void *buffer, u32 size, u32 count, int handle) {
    u32 readArgs[] = {(u32)buffer, size, count, (u32)handle};
    OSMessage message;
    message.message = SERVER_MESSAGE_READ_FILE;
    message.data0 = (u32)readArgs;
    message.data1 = 0x10;
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    return message.data0;
}

bool IsServerFile(const char *path) {

    bool result = false;
    const char *fileList = diiServer_patchFiles;
    if (fileList) {
        u32 fileCount = *(u32 *)fileList; fileList += 4;
        for (u32 i = 0; i < fileCount; i++) {
            u16 pathLength = *(u16 *)fileList; fileList += 2;
            if (strlen(path) == pathLength) {
                if (strncmp(fileList, path, pathLength)) {
                    result = true;
                    break;
                }
            }
            fileList += pathLength;
        }
    }
    log_printf("IsServerFile: %d for %s\n",result,path);
    return result;
}

void OpenServerFile(const char *path, const char *mode, int *handle) {
    int *serverHandle = GetFreeFileHandle();

    OSMessage message;
    message.message = SERVER_MESSAGE_OPEN_FILE;
    message.data0 = (u32)path;
    message.data1 = strlen(path);
    message.data2 = *(u16 *)mode; //A bit of a hack
    OSSendMessage(&diiServer_serverQueue, &message, OS_MESSAGE_BLOCK);
    OSReceiveMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
    *serverHandle = message.data0;
    *handle = message.data0;
}

u32 GetPersistentId(u8 slot) {
    nn_act_Initialize();
    u32 id = nn_act_GetPersistentIdEx(slot);
    nn_act_Finalize();
    return id;
}

int OpenSaveFile(FSClient *client, u8 account, const char *path, const char *mode, int *handle) {
    if (client == &diiServer_fileClient) return -1;

    char newPath[640] = {0};
    if (account == 255) {
        snprintf(newPath, 640, "/vol/save/common/%s", path);
    }
    else {
        snprintf(newPath, 640, "/vol/save/%08X/%s", (unsigned int) GetPersistentId(account), path);
    }
    if (!(IsServerFile(newPath) || mode[0] == 'w')) return -1;
    OpenServerFile(newPath, mode, handle);
    return 0;
}

