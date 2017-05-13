/****************************************************************************
 * Copyright (C) 2016 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "common/retain_vars.h"
#include "coreinit_function_patcher.h"

#include "utils/logger.h"

DECL(void, _Exit, void){
    log_print("_Exit Application closed \n");
    shouldLog = 0;
    real__Exit();
}

DECL(void, __PPCExit, void){
    log_print("__PPCExit Application closed \n");
    shouldLog = 0;
    real___PPCExit();
}

DECL(void, COSVReport, uint32_t module, int level, const char *fmt, va_list *va){
    if(shouldLog == 0)return;
    char * tmp = NULL;
    if((vasprintf(&tmp, fmt, va) >= 0) && tmp)	{
        log_printf("COSVReport module %d level %d: %s",module,level,tmp);
	}
	if(tmp)
		free(tmp);
}


DECL(void, OSLogPrintf, u32 category, u32 level, u32 options, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSLogPrintf category %d level %d options %d: %s\n",category,level,options,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

DECL(void, COSError, uint32_t module, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp)	{
        log_printf("COSError module %d: %s",module,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, COSWarn, uint32_t module, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp)	{
        log_printf("COSWarn module %d: %s",module,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, COSInfo, uint32_t module, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("COSInfo module %d: %s",module,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, COSVerbose, uint32_t module, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("COSVerbose module %d: %s",module,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, OSReport, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSReport: %s",tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

DECL(void, OSReportWarn, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSReportWarn: %s",tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, OSReportInfo, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSReportInfo: %s",tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}


DECL(void, OSReportVerbose, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSReportVerbose: %s",tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

DECL(void, OSPanic, const char *file, int line, const char *fmt, ...){
    if(shouldLog == 0)return;
	char * tmp = NULL;

	va_list va;
	va_start(va, fmt);
	if((vasprintf(&tmp, fmt, va) >= 0) && tmp){
        log_printf("OSPanic [%s line: %d]: %s",file,line,tmp);
	}
	va_end(va);

	if(tmp)
		free(tmp);
}

DECL(void, OSConsoleWrite, const char *msg, uint32_t size){
    if(shouldLog == 0)return;
    char cpy[size+1];
    memcpy(cpy,msg,size);
    cpy[size] = 0;
    log_printf("OSConsoleWrite: %s",cpy);
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(COSVReport,                            LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSLogPrintf,                           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReport,                              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(COSError,                              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(COSWarn,                               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(COSInfo,                               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(COSVerbose,                            LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReportWarn,                          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReportInfo,                          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSReportVerbose,                       LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSPanic,                               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(OSConsoleWrite,                        LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__PPCExit,                             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(_Exit,                                 LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

