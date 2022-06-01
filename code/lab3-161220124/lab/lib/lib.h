#ifndef __lib_h__
#define __lib_h__
#include "types.h"
#define syscall_write 0
#define syscall_fork 1
#define syscall_sleep 2
#define syscall_exit 3

void printf(const char *format,...);
int fork();
void sleep(uint32_t time);
void exit();

#endif
