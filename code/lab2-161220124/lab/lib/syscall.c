#include "lib.h"
#include "types.h"
/*
 * io lib here
 * 库函数写在这
 */

void process_s(char *s);    //process string
void process_c(char c);     //process char
void process_x(uint32_t x); //process unsigned integer
void process_d(uint32_t d); //process signed integer
int32_t sys_write(int fd, char *buf, int len);  //call write()

int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5) 
{
	int32_t ret = 0;

	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80": //num for eax, a1 for ebx, a2 for ecx, a3 for edx
			"=a"(ret):
			"a"(num), "b"(a1), "c"(a2), "d"(a3), "D"(a4), "S"(a5));
		
	return ret;
}

static char *p;
void printf(const char *format,...){
	char buf[400];  //for storing processed string
	p = buf;
	uint32_t para_addr = (uint32_t)&format + 4; //for parameters in printf()
	
	char *form_s;
	char form_c;
	uint32_t form_x;
	int form_d;

	int i;
	for(i = 0; format[i] != '\0'; i++)
	{
		if(format[i] == '%')
		{
			i++;
			if(format[i] == 's')
			{
				form_s = *(char **)para_addr;
				para_addr += 4;
				process_s(form_s);
			}
			else if(format[i] == 'c')
			{
				form_c = *(char *)para_addr;
				para_addr += 4;
				process_c(form_c);
			}
			else if(format[i] == 'x')
			{
				form_x = *(uint32_t *)para_addr;
				para_addr += 4;
				if(form_x == 0)
				{
					*p = '0';
					p++;
				}
				process_x(form_x);
			}
			else if(format[i] == 'd')
			{
				form_d = *(int *)para_addr;
				para_addr += 4;
				if(form_d == 0)
				{
					*p = '0';
					p++;
				}
				if(form_d < 0)
				{
					*p = '-';
					p++;
					form_d = -form_d;
				}
				process_d(form_d);
			}
		}
		else
		{
			*p = format[i];
			p++;
		}
	}
	*p = '\0';
	p++;
	int len = p - buf;
	sys_write(1, buf, len); //1 means stdout
}

void process_s(char *s)
{
	int i;
	for(i = 0; s[i] != '\0'; i++)
	{
		*p = s[i];
		p++;
	}
}

void process_c(char c)
{
	*p = c;
	p++;
}

void process_x(uint32_t x)
{
	if(x == 0)
	{
		return;
	}	
	else
	{
		process_x(x / 16);
		int re = x % 16;
		if(re < 10)
		{
			*p = re + '0';
			p++;
		}
		else
		{
			*p = (re - 10) + 'a';
			p++;
		}
	}
}

void process_d(uint32_t d)
{
	if(d == 0)
	{
		return;
	}	
	else
	{
		process_d(d / 10);
		int re = d % 10;
		*p = re + '0';
		p++;
	}
}

int32_t sys_write(int fd, char *buf, int len)
{
	return syscall(4, (uint32_t)fd, (uint32_t)buf, len, 0, 0); //4 means write
}

