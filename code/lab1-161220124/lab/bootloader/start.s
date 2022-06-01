.code16

.global start
start:

	cli
	inb $0x92, %al
	orb $0x02, %al
	outb %al, $0x92
	data32 addr32 lgdt gdtDesc
	movl %cr0, %eax
	orb $0x01, %al
	movl %eax, %cr0
	data32 ljmp $0x08, $start32 

.code32
start32:
		
	movw $0x0010, %ax   #DS
	movw %ax, %ds

	movw $0x0018, %ax   #GS
	movw %ax, %gs

	movl $0x2000, %esp

	jmp bootMain
	
.p2align 2
gdt:
	.word 0,0             #EMPTY
	.byte 0,0,0,0

	.word 0xffff,0        #CS
	.byte 0,0x9a,0xcf,0

	.word 0xffff,0        #DS
	.byte 0,0x92,0xcf,0

	.word 0xffff,0x8000   #GS 
	.byte 0x0b,0x92,0xcf,0

gdtDesc:
	.word (gdtDesc - gdt -1)
	.long gdt
