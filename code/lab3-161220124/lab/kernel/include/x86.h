#ifndef __X86_H__
#define __X86_H__

#include "x86/cpu.h"
#include "x86/memory.h"
#include "x86/io.h"
#include "x86/irq.h"
#include "x86/pt.h"

void initSeg(void);
void loadUMain(void);

#endif
