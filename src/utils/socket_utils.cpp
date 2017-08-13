#include "socket_utils.h"
#include "../dynamic_libs/socket_functions.h"
#include "diibugger/utils.h"

void sendall(int fd, void *data, int length) {
	int sent = 0;
	while (sent < length) {
		int num = send(fd, data, length - sent, 0);
		CHECK_SOCKET(num, "send");
		sent += num;
		data = (char *)data + num;
	}
}

void recvall(int fd, void *buffer, int length) {
    int bytes = 0;
    while (bytes < length) {
        int num = recv(fd, buffer, length - bytes, 0);
        CHECK_SOCKET(num, "recv");
        bytes += num;
        buffer = (char *)buffer + num;
    }
}

u8 recvbyte(int fd) {
	u8 byte;
	recvall(fd, &byte, 1);
	return byte;
}

u32 recvword(int fd) {
	u32 word;
	recvall(fd, &word, 4);
	return word;
}
