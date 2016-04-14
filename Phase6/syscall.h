// syscall.h

#ifndef _SYSCALL_H_
#define _SYSCALL_H_
#include "typedef.h"

int GetPid();  // no input, 1 return
void Sleep(int);   // 1 input, no return
void StartProc(func_ptr_t);
int SemGet(int);
void SemWait(int);
void SemPost(int);
void MsgSnd(msg_t*);
void MsgRcv(msg_t*);
void TipIRQ3();
#endif
