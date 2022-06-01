#include "boot.h"

#define SECTSIZE 512

void bootMain(void) {
	/* 加载内核至内存，并跳转执行 */
	char *p = (char *)0x8000;   //set loading addr
	int i;
	for(i = 1; i <= 200; i++)     //load sector 1 to 200
		readSect(p + (i - 1) * 512, i);
	struct ELFHeader *elf_header; 
	struct ProgramHeader *ph_beg, *ph_end;
	elf_header = (struct ELFHeader *)p;  //get elf_header
	ph_beg = (struct ProgramHeader *)(p + elf_header->phoff);
	ph_end = ph_beg + elf_header->phnum;  //get first ph & last ph for elf_header 
	struct ProgramHeader *pt;
	for(pt = ph_beg; pt < ph_end; pt++)   //load according to ph's contents
	{
		int j;
		for(j = 0; j < pt->memsz; j++)
		{
			if(j < pt->filesz)
				*(char *)(pt->paddr + j) = *(char *)(p + pt->off + j);
			else
				*(char *)(pt->paddr + j) = 0;
		}
	}
	void (*entry)(void);
	entry = (void *)elf_header->entry;  //get entry_addr from elf_header
	entry();   //jump to next program
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
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
