#ifndef __X86_PT_H__
#define __X86_PT_H__

#define MAX_STACK_SIZE 0x1000
#define MAX_PCB_NUM 20
#define RUN_TIME 1000
#define STATE_RUNNING 0
#define STATE_RUNNABLE 1
#define STATE_BLOCK 2
#define STATE_DEAD 3
#define STATE_IDLE 4
#define USER_SPACE_SIZE 0x100000
#define KERNEL_SPACE_SIZE 0x100000
#define PROCESS_SPACE_SIZE 0x200000
#define syscall_write 0
#define syscall_fork 1
#define syscall_sleep 2
#define syscall_exit 3

extern void initTimer();
extern void initPCB();

extern int s_runnable;
extern int s_block;
extern int s_dead;

struct PTTrapFrame {
    uint32_t irq;                   // 中断号
    uint32_t error;                 // Error Code
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags, esp, ss;
};

struct ProcessTable {
    uint32_t stack[MAX_STACK_SIZE]; // 内核堆栈
    struct PTTrapFrame tf;
    int state;
    int timeCount;
    int sleepTime;
    uint32_t pid;
};

extern struct ProcessTable pcb[MAX_PCB_NUM];
extern struct ProcessTable *now;

#endif
