// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void TimerISR();
void GetPidISR();
void Sleep_ISR(int);
void StartProcISR(int,int);
void SemGetISR(int);
void SemPostISR(int);
void SemWaitISR(int);
void MsgSndISR(int);
void MsgRcvISR(int);
#endif
