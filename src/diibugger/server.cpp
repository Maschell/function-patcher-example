#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "common/c_retain_vars.h"
#include "diibugger/debugger.h"
#include "diibugger/server.h"
#include "utils/socket_utils.h"
#include "utils/logger.h"
#include "system/OSThread.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/nn_save_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/nn_act_functions.h"

void InitDiibugger(){
    log_printf("InitDiibugger()[LINE %d]: In InitDiibugger\n",__LINE__);
    diiServer_connection = false;
    diiServer_stepState = STEP_STATE_RUNNING;

    FSInit();
    log_printf("InitDiibugger()[LINE %d]: FSInit() done\n",__LINE__);
    SAVEInit();
    log_printf("InitDiibugger()[LINE %d]: SAVEInit() done\n",__LINE__);
    FSInitCmdBlock(&diiServer_fileBlock);
    log_printf("InitDiibugger()[LINE %d]: FSInitCmdBlock(%08X) done\n",__LINE__,&diiServer_fileBlock);
    s32 res = FSAddClient(&diiServer_fileClient, -1);
    log_printf("InitDiibugger()[LINE %d]: FSAddClient() done %d\n",__LINE__,res);

    log_printf("InitDiibugger()[LINE %d]: FS Stuff done\n",__LINE__);

	OSInitMessageQueue(&diiServer_serverQueue, diiServer_serverMessages, MESSAGE_COUNT);
	OSInitMessageQueue(&diiServer_clientQueue, diiServer_clientMessages, MESSAGE_COUNT);
	OSInitMessageQueue(&diiServer_fileQueue, &diiServer_fileMessage, 1);

	log_printf("InitDiibugger()[LINE %d]: OSInitMessageQueue done\n",__LINE__);

	 diiServer_threadPointer = CThread::create(RPCServer, NULL, CThread::eAttributeAffCore2 | CThread::eAttributePinnedAff, 12);
	 log_printf("InitDiibugger()[LINE %d]: Thread created. %08X\n",__LINE__,diiServer_threadPointer);
	 /*

pThread->resumeThread();

	s32 res = OSCreateThread(
		&diiServer_thread,
		RPCServer,
		0,
		0,
		(u32)((u32)diiServer_stack + STACK_SIZE),
		STACK_SIZE,
		0,
		12
	);*/
	diiServer_threadPointer->setThreadName("Debug Server");
	log_printf("InitDiibugger()[LINE %d]: Thread name set.\n",__LINE__);
	//res = OSResumeThread(&diiServer_thread);
    diiServer_threadPointer->resumeThread();
	log_printf("InitDiibugger()[LINE %d]: Thread started.\n",__LINE__);

	//log_printf("InitDiibugger()[LINE %d]: Thread started \n",__LINE__);

	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_DSI, DSIHandler_Fatal);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_ISI, ISIHandler_Fatal);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_PROGRAM, ProgramHandler_Initialize);

	log_printf("InitDiibugger()[LINE %d]: Set the ExceptionCallbacks\n",__LINE__);
}

void RPCServer(CThread *thread, void *arg) {
    log_printf("RPCServer()[LINE %d]: RPC Server started! \n",__LINE__);
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_THREAD, OS_EXCEPTION_DSI, DSIHandler_Fatal);
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_THREAD, OS_EXCEPTION_ISI, ISIHandler_Fatal);
    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_THREAD, OS_EXCEPTION_PROGRAM, ProgramHandler_Fatal);
    log_printf("RPCServer()[LINE %d]: Exception Callbacks set! \n",__LINE__);

	s32 result = socket_lib_init();
	CHECK_SOCKET(result, "socket_lib_init");

	log_printf("RPCServer()[LINE %d]: socket lib init done! \n",__LINE__);

	while (true) {

        int socketVal = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        CHECK_SOCKET(socketVal, "socket");
        log_printf("RPCServer()[LINE %d]: created socket : %d \n",__LINE__,socketVal);

        u32 reuseaddr = 1;
        result = setsockopt(socketVal, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, 4);
        CHECK_SOCKET(result, "setsockopt");

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = 1559;
        serverAddr.sin_addr.s_addr = 0;
        result = bind(socketVal, (sockaddr *)&serverAddr, 16);
        CHECK_SOCKET(result, "bind");
        log_printf("RPCServer()[LINE %d]: bind result : %d \n",__LINE__,result);

        result = listen(socketVal, 1);
        CHECK_SOCKET(result, "listen");
        log_printf("RPCServer()[LINE %d]: listen result : %d \n",__LINE__,result);

        s32 length = 16;
        s32 client = accept(socketVal, (sockaddr *)&serverAddr, &length);
        CHECK_SOCKET(client, "accept");
        log_printf("RPCServer()[LINE %d]: accept result : %d \n",__LINE__,client);

        OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_DSI, DSIHandler_Debug);
	    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_ISI, ISIHandler_Debug);
	    OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_PROGRAM, ProgramHandler_Debug);
	    log_printf("RPCServer()[LINE %d]: Set debug exceptions\n",__LINE__);
	    diiServer_connection = true;
        log_printf("RPCServer()[LINE %d]: Set diiServer_connection to %d\n",__LINE__,diiServer_connection);
        while (true) {
            log_printf("RPCServer()[LINE %d]: Waiting for a command\n",__LINE__);
            u8 cmd = recvbyte(client);
            log_printf("RPCServer()[LINE %d]: Got the command %d\n",__LINE__,cmd);
            if (cmd == 1) { //Close
                //Remove all breakpoints
                for (int i = 0; i < 10; i++) {
                    if (diiServer_breakpoints[i].address) {
                        WriteCode(diiServer_breakpoints[i].address, diiServer_breakpoints[i].instruction);
                        diiServer_breakpoints[i].address = 0;
                        diiServer_breakpoints[i].instruction = 0;
                    }
                }

                //Remove file patches
                if (diiServer_patchFiles) {
                    free(diiServer_patchFiles);
                    diiServer_patchFiles = 0;
                }

                //Make sure we're not stuck in an exception when the
                //debugger disconnects without handling it
                if (diiServer_crashState == CRASH_STATE_BREAKPOINT) {
                    OSMessage message;
                    message.message = CLIENT_MESSAGE_CONTINUE;
                    OSSendMessage(&diiServer_clientQueue, &message, OS_MESSAGE_BLOCK);
                    //Wait until execution is resumed before installing the OSFatal crash handler
                    while (diiServer_crashState != CRASH_STATE_NONE) {
                        OSSleepTicks(100000);
                    }
                }

                diiServer_connection = false;
            	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_DSI, DSIHandler_Fatal);
	            OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_ISI, ISIHandler_Fatal);
	            OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_PROGRAM, ProgramHandler_Fatal);
                break;
            }

            else if (cmd == 2) { //Read
                u32 addr = recvword(client);
                u32 num = recvword(client);

                //Restore instructions, we don't want to see "trap" in the disassembly
                breakpoint *bp = GetBreakPointRange(addr, num, 0);
                while (bp) {
                    WriteCode(bp->address, bp->instruction);
                    bp = GetBreakPointRange(addr, num, bp);
                }

                sendall(client, (void *)addr, num);

                //Now put back the breakpoints
                bp = GetBreakPointRange(addr, num, 0);
                while (bp) {
                    WriteCode(bp->address, TRAP);
                    bp = GetBreakPointRange(addr, num, bp);
                }
            }

            else if (cmd == 3) { //Write
                u32 addr = recvword(client);
                u32 num = recvword(client);
                recvall(client, (void *)addr, num);
            }

            else if (cmd == 4) { //Write code
                u32 addr = recvword(client);
                u32 instr = recvword(client);

                //Make sure we don't overwrite breakpoint traps
                breakpoint *bp = GetBreakPoint(addr, 12);
                if (bp) bp->instruction = instr;
                else WriteCode(addr, instr);
            }

            else if (cmd == 5) { //Get thread list
                //Might need OSDisableInterrupts here?
                char buffer[0x1000]; //This should be enough
                u32 offset = 0;

                OSThread *currentThread = OSGetCurrentThread();
                OSThread *iterThread = currentThread;
                OSThreadLink threadLink;
                do { //Loop previous threads
                    offset += PushThread(buffer, offset, iterThread);
                    OSGetActiveThreadLink(iterThread, &threadLink);
                    iterThread = threadLink.prev;
                } while (iterThread);

                OSGetActiveThreadLink(currentThread, &threadLink);
                iterThread = threadLink.next;
                while (iterThread) { //Loop next threads
                    offset += PushThread(buffer, offset, iterThread);
                    OSGetActiveThreadLink(iterThread, &threadLink);
                    iterThread = threadLink.next;
                }

                sendall(client, &offset, 4);
                sendall(client, buffer, offset);
            }

            else if (cmd == 6) { //Push message
                OSMessage message;
                recvall(client, &message, sizeof(OSMessage));
                OSSendMessage(&diiServer_clientQueue, &message, OS_MESSAGE_BLOCK);
            }

            else if (cmd == 7) { //Get messages
                OSMessage messages[10];
                u32 count = 0;

                OSMessage message;
                while (OSReceiveMessage(&diiServer_serverQueue, &message, OS_MESSAGE_NOBLOCK)) {
                    messages[count] = message;
                    count++;
                }

                sendall(client, &count, 4);
                for (u32 i = 0; i < count; i++) {
                    sendall(client, &messages[i], sizeof(OSMessage));
                    if (messages[i].data0) {
                        sendall(client, (void *)messages[i].data0, messages[i].data1);
                    }
                }
            }

            else if (cmd == 8) { //Get stack trace
                u32 sp = diiServer_crashContext.gpr[1];
                u32 index = 0;
                u32 stackTrace[30];
                while (isValidStackPtr(sp)) {
                    sp = *(u32 *)sp;
                    if (!isValidStackPtr(sp)) break;

                    stackTrace[index] = *(u32 *)(sp + 4);
                    index++;
                }

                sendall(client, &index, 4);
                sendall(client, stackTrace, index * 4);
            }

            else if (cmd == 9) { //Poke registers
                recvall(client, &diiServer_crashContext.gpr, 4 * 32);
                recvall(client, &diiServer_crashContext.fpr, 8 * 32);
            }

            else if (cmd == 10) { //Toggle breakpoint
                u32 address = recvword(client);
                breakpoint *bp = GetBreakPoint(address, 10);
                if (bp) {
                    WriteCode(address, bp->instruction);
                    bp->address = 0;
                    bp->instruction = 0;
                }
                else {
                    bp = GetFreeBreakPoint();
                    bp->address = address;
                    bp->instruction = *(u32 *)address;
                    WriteCode(address, TRAP);
                }
            }

            else if (cmd == 11) { //Read directory
                log_printf("RPCServer()[LINE %d] Readdir: Start\n",__LINE__);
                char path[640] = {0}; //512 + 128
                u32 pathlen = recvword(client);
                log_printf("RPCServer()[LINE %d] Readdir: pathlength %d\n",__LINE__,pathlen);
                if (pathlen < 640) {
                    recvall(client, &path, pathlen);
                    s32 error;
                    s32 handle;
                    FSDirEntry entry;
                     log_printf("RPCServer()[LINE %d] FSOpenDir(0x%08X, %08X, %s, %08X, -1)\n",__LINE__,&diiServer_fileClient,&diiServer_fileBlock,path,&handle);
                    error = FSOpenDir(&diiServer_fileClient, &diiServer_fileBlock, path, &handle, -1);
                    log_printf("RPCServer()[LINE %d] Readdir: open dir res: handle : %d \n",__LINE__,handle);
                    //CHECK_ERROR(error, "FSOpenDir");


                    while (FSReadDir(&diiServer_fileClient, &diiServer_fileBlock, handle, &entry, -1) == 0) {
                        s32 namelen = strlen(entry.name);
                        sendall(client, &namelen, 4);
                        sendall(client, &entry.stat.flag, 4);
                        if (!(entry.stat.flag & 0x80000000)) {
                            sendall(client, &entry.stat.size, 4);
                        }
                        sendall(client, &entry.name, namelen);
                    }

                    error = FSCloseDir(&diiServer_fileClient, &diiServer_fileBlock, handle, -1);
                    CHECK_ERROR(error, "FSCloseDir");
                }
                s32 terminator = 0;
                sendall(client, &terminator, 4);
            }

            else if (cmd == 12) { //Dump file
                char path[640] = {0};
                u32 pathlen = recvword(client);
                if (pathlen < 640) {
                    recvall(client, &path, pathlen);

                    s32 error, handle;
                    error = FSOpenFile(&diiServer_fileClient, &diiServer_fileBlock, path, "r", &handle, -1);
                    CHECK_ERROR(error, "FSOpenFile");

                    FSStat stat;
                    error = FSGetStatFile(&diiServer_fileClient, &diiServer_fileBlock, handle, &stat, -1);
                    CHECK_ERROR(error, "FSGetStatFile");

                    u32 size = stat.size;
                    sendall(client, &stat.size, 4);

                    char *buffer = (char *)memalign(0x40,0x20000);

                    u32 read = 0;
                    while (read < size) {
                        int num = FSReadFile(&diiServer_fileClient, &diiServer_fileBlock, buffer, 1, 0x20000, handle, 0, -1);
                        CHECK_ERROR(num, "FSReadFile");
                        read += num;

                        sendall(client, buffer, num);
                    }

                    error = FSCloseFile(&diiServer_fileClient, &diiServer_fileBlock, handle, -1);
                    CHECK_ERROR(error, "FSCloseFile");

                    free(buffer);
                }
                else {
                    OSFatal("pathlen >= 640");
                }
            }

            else if (cmd == 13) { //Get module name
                char name[100] = {0};
                s32 length = 100;
                OSDynLoad_GetModuleName(-1, name, &length);

                length = strlen(name);
                sendall(client, &length, 4);
                sendall(client, name, length);
            }

            else if (cmd == 14) { //Set patch files
                if (diiServer_patchFiles) {
                    free(diiServer_patchFiles);
                }

                s32 length = recvword(client);
                diiServer_patchFiles = (char *)memalign(4,length);
                recvall(client, diiServer_patchFiles, length);
            }

            else if (cmd == 15) { //Send file message
                OSMessage message;
                recvall(client, &message, sizeof(OSMessage));
                OSSendMessage(&diiServer_fileQueue, &message, OS_MESSAGE_BLOCK);
            }

            else if (cmd == 16) { //Clear patch files
                if (diiServer_patchFiles) {
                    free(diiServer_patchFiles);
                    diiServer_patchFiles = 0;
                }
            }

            else if (cmd == 17) { //Get persistent id
                nn_act_Initialize();
                u8 slot = nn_act_GetSlotNo();
                u32 persistentId = nn_act_GetPersistentIdEx(slot);
                sendall(client, &persistentId, 4);
                nn_act_Finalize();
            }
        }

        CHECK_SOCKET(socketclose(client), "socketclose");
        CHECK_SOCKET(socketclose(socketVal), "socketclose");
    }

    //SOFinish();
    //return 0;
}
