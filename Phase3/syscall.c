// syscall.c
// software interrupt/syscalls, i.e., kernel service API
#include "extern.h"

int GetPid() {                   // no input, has return
   int pid;

   asm("int $48; movl %%eax, %0" // CPU inst
       : "=g" (pid)              // output from asm("...")
       :                         // no input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after

   return pid;
}

void Sleep(int seconds) {        // has input, no return

   asm("movl %0, %%eax; int $49" // CPU inst
       :                         // no output from asm("...")
       : "g" (seconds)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

void StartProc(func_ptr_t ptr) {        // has input, no return

   asm("movl %0, %%eax; int $50" // CPU inst
       :                         // no output from asm("...")
       : "g" (ptr)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

int SemGet(int semNum) {    //input and return               
   int sem_id;

   asm("int $51; movl %%eax, %0" // CPU inst
       : "=g" (semNum)              // output from asm("...")
       :                         // no input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after

   return sem_id;
}

void SemWait(int sem_id) {        // has input, no return

   asm("movl %0, %%eax; int $52" // CPU inst
       :                         // no output from asm("...")
       : "g" (sem_id)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

void SemPost(int sem_id) {        // has input, no return

   asm("movl %0, %%eax; int $49" // CPU inst
       :                         // no output from asm("...")
       : "g" (sem_id)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

