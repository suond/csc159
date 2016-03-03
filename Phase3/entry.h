// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32
#define GETPID_INTR 48
#define SLEEP_INTR 49
#define STARTPROC_INTR 50
#define SEMGET_INTR 51
#define SEMWAIT_INTR 52
#define SEMPOST_INTR 53
#define KERNEL_CODE 0x08         // kernel's code segment
#define KERNEL_DATA 0x10         // kernel's data segment
#define KERNEL_STACK_SIZE 32768  // kernel's stack byte size


// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

extern void LoadRun();           // code defined in entry.S
extern void TimerEntry();        // code defined in entry.S
extern void GetPidEntry();
extern void SleepEntry();
extern void StartProcEntry();
extern void SemGetEntry();
extern void SemWaitEntry();
extern void SemPostEntry();

__END_DECLS

#endif

#endif
