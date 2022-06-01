#include "x86.h"
#include "common.h"
#include "device.h"

extern SegDesc gdt[NR_SEGMENTS];
extern TSS tss;
extern void readSect(void *dst, int offset);
int s_runnable;
int s_block;
int s_dead;
struct ProcessTable pcb[MAX_PCB_NUM];
struct ProcessTable *now;
void initPCB();   //initialize  PCBs
int index_dead(); //find the head of dead line
void IDLE_now();  //IDLE process
void succeed_father_regs(struct ProcessTable *p);  //copy process info
void update_s();  //update s_block & s_runnnable
int fork();   
void sleep(uint32_t time);
void exit();
void manage();

void initPCB()
{
	s_runnable = -1;
	s_block = -1;
	s_dead = 1;
	now = NULL;

	int i;
	for(i = 0; i < MAX_PCB_NUM; i++)
		pcb[i].state = STATE_DEAD;
		
	int user_index = index_dead();
	if(user_index == -1)
		assert(0);
	struct ProcessTable *pt = &pcb[user_index];
	pt->state = STATE_RUNNABLE;
	pt->timeCount = RUN_TIME;
	pt->sleepTime = 0;
	pt->pid = user_index;
	
	char *p = (char *)0x8000;
	readSect(p, 201);
	struct ELFHeader *elf_head = (struct ELFHeader *)p;
	uint32_t entry = elf_head->entry;

	pt->tf.gs = KSEL(SEG_SHOW);
	pt->tf.fs = USEL(SEG_UDATA);
	pt->tf.es = USEL(SEG_UDATA);
	pt->tf.ds = USEL(SEG_UDATA);
	pt->tf.ss = USEL(SEG_UDATA);
	pt->tf.cs = USEL(SEG_UCODE);
	pt->tf.esp = 0x200000 + USER_SPACE_SIZE - 0x4;
	pt->tf.ebp = 0x200000 + USER_SPACE_SIZE - 0x4;
	pt->tf.eflags = 0x202;	
	pt->tf.eip = entry;
	
	s_runnable = 1;

	pt = &pcb[0];
	pt->state = STATE_IDLE;
	pt->sleepTime = 0;
	
	pt->tf.gs = KSEL(SEG_SHOW);
	pt->tf.fs = KSEL(SEG_KDATA);
	pt->tf.es = KSEL(SEG_KDATA);
	pt->tf.ds = KSEL(SEG_KDATA);
	pt->tf.ss = KSEL(SEG_KDATA);
	pt->tf.cs = KSEL(SEG_KCODE);
	pt->tf.esp = 0x200000 - 0x4;
	pt->tf.ebp = 0x200000 - 0x4;
	pt->tf.eflags = 0x202;	
	void (*entry_IDLE)();
	entry_IDLE = IDLE_now;
	pt->tf.eip = (uint32_t)entry_IDLE + 3;
}

int index_dead()
{
	if(s_dead == -1)
		return -1;
	int record = s_dead;
	s_dead = (s_dead + 1) % (MAX_PCB_NUM);
	while(pcb[s_dead].state != STATE_DEAD)
	{
		if(s_dead == record)
		{
			s_dead = -1;
			return -1;
		}
		s_dead = (s_dead + 1) % MAX_PCB_NUM;
	}
	return record;
}

void IDLE_now()
{
	while(1)
		waitForInterrupt();
}

int fork()
{
	int index = index_dead();
	struct ProcessTable *pt = &pcb[index];
	pt->state = STATE_RUNNABLE;
	pt->pid = index;
	succeed_father_regs(pt);
	uint32_t father = 0x200000 + (now->pid - 1) * PROCESS_SPACE_SIZE;
	uint32_t child = 0x200000 + (pt->pid - 1) * PROCESS_SPACE_SIZE;
	
	int i;
	for(i = 0; i < USER_SPACE_SIZE; i++)
	{
		*(char *)(child + i) = *(char *)(father + i);
	}
	update_s();
	return index;
}

void succeed_father_regs(struct ProcessTable *p)
{
	p->tf.edi = now->tf.edi;
	p->tf.esi = now->tf.esi;
	p->tf.xxx = now->tf.xxx;
	p->tf.ebx = now->tf.ebx;
	p->tf.edx = now->tf.edx;
	p->tf.ecx = now->tf.ecx;
	p->tf.eax = 0;
	p->tf.cs = now->tf.cs;
	p->tf.eflags = now->tf.eflags;
	p->tf.ss = now->tf.ss;
	p->tf.ds = now->tf.ds;
	p->tf.es = now->tf.es;
	p->tf.fs = now->tf.fs;
	p->tf.gs = now->tf.gs;
	p->tf.eip = now->tf.eip;
	p->tf.ebp = now->tf.ebp;
	p->tf.esp = now->tf.esp;
}

void update_s()
{
	if(s_runnable == -1)
		s_runnable = 0;
	int record = s_runnable;
	while(record != -1)
	{
		if(pcb[record].state == STATE_RUNNABLE)
		{
			s_runnable = record;
			break;
		}
		record = (record + 1) % MAX_PCB_NUM;
		if(record == s_runnable)
		{
			s_runnable = -1;
			break;
		}
	}
	
	if(s_block == -1)
		s_block = 0;
	record = s_block;
	while(record != -1)
	{
		if(pcb[record].state == STATE_BLOCK)
		{
			s_block = record;
			break;
		}
		record = (record + 1) % MAX_PCB_NUM;
		if(record == s_block)
		{
			s_block = -1;
			break;
		}
	}
}

void sleep(uint32_t time)
{
	now->sleepTime = time;
	now->state = STATE_BLOCK;
	update_s();
}

void exit()
{
	now->state = STATE_DEAD;
	now = NULL;
	update_s();
}

void manage()
{
	if(now && now->state == STATE_BLOCK)
	{
		now = NULL;
	}
	if(now && now->timeCount == 0 && now->state != STATE_IDLE)
	{
		now->state = STATE_RUNNABLE;
		now = NULL;
		int record = s_runnable;
		while(record != -1)
		{
			record = (record + 1) % MAX_PCB_NUM;
			if(record == s_runnable)
				break;
			if(pcb[record].state == STATE_RUNNABLE)
				break;
		}
		s_runnable = record;
	}
	int pos = s_block;
	while(pos != -1)
	{
		if(pcb[pos].state == STATE_BLOCK)
		{
			if(pcb[pos].sleepTime == 0)
			{
				pcb[pos].state = STATE_RUNNABLE;
			}
		}
		pos = (pos + 1) % MAX_PCB_NUM;
		if(pos == s_block)
			break;
	}
	if(now && now->state == STATE_RUNNING)
	{
		update_s();
		return;
	}
	if(s_runnable == -1)    //no runnnable, so do IDLE
	{
		if(now->state == STATE_IDLE)
		{
			now->timeCount = 1;
			update_s();
			return;
		}
		now = &pcb[0];
		now->state = STATE_IDLE;
		now->timeCount = 1;
		tss.esp0 = (uint32_t)&(now->state);
		update_s();
		return;
	}
	now = &pcb[s_runnable];   //choose runnnable process to do
	now->timeCount = RUN_TIME;
	now->state = STATE_RUNNING;
	tss.esp0 = (uint32_t)&(now->state);
	update_s();
	if(now->state != STATE_IDLE)    //change SEG info
	{
		uint32_t offset = (now->pid - 1) * PROCESS_SPACE_SIZE;
		gdt[SEG_UCODE] = SEG(STA_X | STA_R, offset,       0xffffffff, DPL_USER);
		gdt[SEG_UDATA] = SEG(STA_W,         offset,       0xffffffff, DPL_USER);
		setGdt(gdt, sizeof(gdt));
	}
}

