// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32
#define GETPID_INTR 48
#define SLEEP_INTR 49
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

__END_DECLS

#endif

#endif
