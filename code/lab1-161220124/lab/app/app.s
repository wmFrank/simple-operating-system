.code32

.global start
start:
	movl $((80*8+0)*2), %edi             
	movb $0x0c, %ah                         
	movb $72, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi
	movb $101, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $108, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $108, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $111, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $44, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $32, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $87, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $111, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $114, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $108, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $100, %al                           
	movw %ax, %gs:(%edi)
	addl $2, %edi  
	movb $33, %al                           
	movw %ax, %gs:(%edi)

loop:
	jmp loop

