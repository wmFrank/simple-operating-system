#include "x86.h"
#include "device.h"

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void TimeHandle(struct TrapFrame *tf);

void irq0_c(struct TrapFrame *tf);  //process 0-c interrupt

int print_on_screen(char *str, int len);  //print str to the screen

void put_char(char ch);  //put ch to the screen

extern void sleep(unsigned time);

extern int fork();

extern void exit();

extern void manage();

uint32_t irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */

	switch(tf->irq) {
		case -1:
			break;
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
			irq0_c(tf);
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
			TimeHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}

	if(now == NULL || now->state == STATE_BLOCK) //no process or block, need manage
		manage();

	uint32_t esp;
	esp = (uint32_t)&(now->tf);   //return the needed process's esp
	return esp;
}

void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch(tf->eax)
	{
		case syscall_write:  //sys_write matches number 4        
			now->tf.eax = print_on_screen((char *)tf->ecx, tf->edx); //ecx record addr of str, edx record len of str 
			break;
		case syscall_fork:
			now->tf.eax = fork();
			break;
		case syscall_sleep:
			sleep((uint32_t)(tf->ebx));
			break;
		case syscall_exit:
			exit();
			break;
		default:
			assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void irq0_c(struct TrapFrame *tf)
{
	assert(0);   //no chance to use, so stop
	return;
}

void TimeHandle(struct TrapFrame *tf)
{
	if(now != NULL)
		now->timeCount--;
	int record = s_block;
	while(record != -1)
	{
		if(pcb[record].state == STATE_BLOCK)
		{
			pcb[record].sleepTime--;
		}
		record = (record + 1) % MAX_PCB_NUM;
		if(record == s_block)
			break;
	}
	manage();
}

#define screen_width 80
#define screen_height 30
int cur = 0;     //record position of cursor

int print_on_screen(char *str, int len)
{
	if(now->pid == 2 && (int)str > 0x200000)
	{
		str+= 0x200000;
	}
	while(*str != '\0')
	{
		put_char(*str);  //each time print one ch
		str++;
	}
	return len;
}

void put_char(char ch)
{
	short *addr = (short *)0xB8000;   //GS
	if(ch == '\n')
		cur = (cur / screen_width + 1) * screen_width;  //switch line
	else
	{
		*(addr + cur) = 0x0c00 + ch;  //0c means light red
		cur++;
	}
}

void clear_screen()  //to clear the screen
{
	short *addr = (short *)0xB8000;
	int i;
	for(i = 0; i < screen_width * screen_height; i++)
	{
		*(addr + i) = 0;
	}
	cur = 0;
}


