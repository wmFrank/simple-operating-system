#ifndef __lib_h__
#define __lib_h__
#include "types.h"
#define syscall_write 0
#define syscall_fork 1
#define syscall_sleep 2
#define syscall_exit 3
#define syscall_sem_init 4
#define syscall_sem_post 5
#define syscall_sem_wait 6
#define syscall_sem_destroy 7

void printf(const char *format,...);
int fork();
void sleep(uint32_t time);
void exit();

int sem_init(sem_t *sem, uint32_t value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);

#endif
