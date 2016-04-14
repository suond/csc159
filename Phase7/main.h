// main.h, 159

#ifndef _MAIN_H_
#define _MAIN_H_
#include "typedef.h"

int main();
void SetEntry(int,func_ptr_t);
void InitKernelData();
void InitKernelControl();
void Scheduler();
void KernelMain();

#endif
