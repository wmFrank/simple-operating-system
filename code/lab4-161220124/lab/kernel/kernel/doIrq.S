.code32

.global irqEmpty
irqEmpty:
	pushl $0 // push dummy error code
	pushl $-1 // push interrupt vector into kernel
	jmp asmDoIrq

.global irq0
irq0:
	pushl $0x0
	jmp asmDoIrq

.global irq1
irq1:
	pushl $0x1
	jmp asmDoIrq

.global irq2
irq2:
	pushl $0x2
	jmp asmDoIrq

.global irq3
irq3:
	pushl $0x3
	jmp asmDoIrq

.global irq4
irq4:
	pushl $0x4
	jmp asmDoIrq

.global irq5
irq5:	
	pushl $0x5
	jmp asmDoIrq

.global irq6
irq6:
	pushl $0x6
	jmp asmDoIrq

.global irq7
irq7:
	pushl $0x7
	jmp asmDoIrq

.global irq8
irq8:
	pushl $0x8
	jmp asmDoIrq

.global irq9
irq9:
	pushl $0x9
	jmp asmDoIrq

.global irqa
irqa:
	pushl $0xa
	jmp asmDoIrq

.global irqb
irqb:
	pushl $0xb
	jmp asmDoIrq

.global irqc
irqc:
	pushl $0xc
	jmp asmDoIrq

.global irqGProtectFault
irqGProtectFault:
	pushal
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs	

	pushl $0xd
	jmp asmDoIrq

.global irqtime
irqtime:
	pushal
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs	

	pushl $0
	pushl $0x20
	jmp asmDoIrq

.global irqSyscall
irqSyscall:
	pushal
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs	

	pushl $0 // push dummy error code
	pushl $0x80 // push interrupt vector into kernel stack
	jmp asmDoIrq

.global asmDoIrq
asmDoIrq:
	cli 
	pushal // push process state into kernel stack
	pushl %esp
	movw $16, %ax 
	movw %ax, %ds 
	call irqHandle
	add $36, %esp  
	add $4, %esp 
	cmpl %eax, %esp  
	je FLAG  
	pushl %eax 
	popl %esp  
FLAG:
	add $4, %esp 
	add $4, %esp 
	popl %gs  
	popl %fs  
	popl %es  
	popl %ds  
	popal     
	sti   
	iret  
	

	//addl $4, %esp
	//popal
	//addl $4, %esp //interrupt vector is on top of kernel stack
	//addl $4, %esp //error code is on top of kernel stack
	//iret



