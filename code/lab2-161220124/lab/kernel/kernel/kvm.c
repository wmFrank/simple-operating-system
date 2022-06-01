#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_SHOW] = SEG(STA_W,         0xB8000,    0xffffffff, DPL_KERN); //add GS for print on screen
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */
	tss.esp0 = 0x200000 - 0x4;  //according to ppt
	tss.ss0 = KSEL(SEG_KDATA);  //kernal tss
	
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	asm volatile("movw %%ax, %%es":: "a" (KSEL(SEG_KDATA))); //init k_es
	asm volatile("movw %%ax, %%ds":: "a" (KSEL(SEG_KDATA))); //init k_ds
	asm volatile("movw %%ax, %%ss":: "a" (KSEL(SEG_KDATA))); //init k_ss
	asm volatile("movw %%ax, %%gs":: "a" (KSEL(SEG_SHOW)));  //init gs

	lLdt(0);
	
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
	asm volatile("sti");                               // open interrupt
	asm volatile("pushl %%eax"::"a"(USEL(SEG_UDATA))); //push u_ss
	asm volatile("movl $0x800000, %eax");              //push esp
	asm volatile("pushl %eax");
	asm volatile("pushfl");                            //push eflags
	asm volatile("pushl %%eax"::"a"(USEL(SEG_UCODE))); //push u_cs
	asm volatile("pushl %0"::"r"(entry));              // push eip
	
	asm volatile("movw %%ax, %%es":: "a" (KSEL(SEG_UDATA))); //set u_es
	asm volatile("movw %%ax, %%ds":: "a" (KSEL(SEG_UDATA))); //set u_ds

	asm volatile("iret"); //pop  eip & u_cs & eflags & esp & u_ss
}

void loadUMain(void) {

	/*加载用户程序至内存*/
	clear_screen();
	char *p = (char *)0x8000;   //set loading addr
	readSect(p, 201);     //load sector 201
	struct ELFHeader *elf_header; 
	struct ProgramHeader *ph_beg, *ph_end;
	elf_header = (struct ELFHeader *)p;  //get elf_header
	ph_beg = (struct ProgramHeader *)(p + elf_header->phoff);
	ph_end = ph_beg + elf_header->phnum;  //get first ph & last ph for elf_header 
	int num = 1; //number of sectors
	struct ProgramHeader *pt;
	for(pt = ph_beg; pt < ph_end; pt++)   //load according to ph's content
	{
		int j;
		for(j = 0; j < pt->memsz; j++)
		{
			if(j < pt->filesz)
			{
				if((pt->off + j) / 512 > num - 1)
				{
					readSect(p + num * 512, num + 201);
					j--;
					num++;
					continue;
				}
					*(char *)(pt->paddr + j) = *(char *)(p + pt->off + j);	
			}
			else
				*(char *)(pt->paddr + j) = 0;
		}
	}
	enterUserSpace(elf_header->entry); //jump to User space 
}

