#ifndef _SOCKET_UTILS_H_
#define _SOCKET_UTILS_H_

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

void sendall(int fd, void *data, int length);

void recvall(int fd, void *buffer, int length);

u8 recvbyte(int fd);

u32 recvword(int fd);

#ifdef __cplusplus
}
#endif

#endif /* _SOCKET_UTILS_H_ */
