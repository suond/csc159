// main.c, 159
// the kernel is simulated, not yet real
//
// Team Name: NULL (Members: David Suon and LLoyd Foreman)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "toolfunc.h"   // handy functions for Kernel
#include "proc.h"       // processes such as IdleProc()
#include "typedef.h"    // data types
#include "entry.h"




// kernel data stuff:
int running_pid, OS_clock;            // currently-running PID, if -1, none running
q_t ready_q, free_q, sleep_q, sem_q;        // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC_NUM];    // process table
char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes
sem_t sem[Q_LEN]; //array of semaphores
typedef void (* func_ptr)();
struct i386_gate *IDT_ptr;
msg_q_t msg_q[MAX_PROC_NUM];

//for testing only
int product_sem_id, product_count;



int main() {
   int pid;
   
   InitKernelData();
   InitKernelControl();
   pid = DeQ(&free_q);
   StartProcISR(pid,(unsigned int)IdleProc);
   pid = DeQ(&free_q);
   StartProcISR(pid,(unsigned int)InitProc);
   //running_pid = -1;
   //Scheduler();
   LoadRun(pcb[0].TF_ptr);
   
   return 0;   // not reached, but compiler needs it for syntax
}

void SetEntry(int entry_num, func_ptr_t func_ptr) {
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int) func_ptr, get_cs(), ACC_INTR_GATE,0);

}
void InitKernelData() {
   int i;
   MyBzero((char*)&sleep_q,sizeof (q_t));
   MyBzero((char*)&ready_q,sizeof(q_t)); //clear queues
   MyBzero((char*)&free_q,sizeof(q_t));
   MyBzero((char*)&sem_q,sizeof(q_t));
   
   for(i=0; i<MAX_PROC_NUM;i++){
	MyBzero((char*)&msg_q[i],sizeof(msg_q_t));
   }

   //loop number i from 0 to 19:
   for(i=0; i<Q_LEN; i++){
      EnQ(i,&free_q);
      EnQ(i,&sem_q);
      MyBzero((char*)&pcb[i],sizeof(pcb_t)); 
   }
	OS_clock = 0;
   running_pid = 0; 
   product_count = 0; 
}

void InitKernelControl() {
	IDT_ptr = get_idt_base();
	SetEntry(32, TimerEntry);
	SetEntry(48, GetPidEntry);	
	SetEntry(49, SleepEntry);
        SetEntry(50, StartProcEntry);
	SetEntry(51, SemGetEntry);	
	SetEntry(52, SemWaitEntry);
	SetEntry(53, SemPostEntry);
	SetEntry(54, MsgSndEntry);
	SetEntry(55, MsgRcvEntry);
	outportb(0x21, ~0x01);  //masking the PIC
	
}

void Scheduler() {  // to choose running PID
   if(running_pid >0)
      return;

   if(running_pid == 0)
      pcb[running_pid].state = READY;

   running_pid = DeQ(&ready_q);
   if(running_pid == -1)
      running_pid=0;

   pcb[running_pid].state = RUN;
   
}

void KernelMain(TF_t *TF_ptr) {
   
  
   int new_pid;
   
	pcb[running_pid].TF_ptr = TF_ptr;
	switch(TF_ptr ->intr_id) {
	    case(TIMER_INTR):
	       TimerISR();
		OS_clock++;
		checkWait();
		outportb(0x20, 0x60); //stops the timer pic with a code
	       break;
		case(SLEEP_INTR):
		   Sleep_ISR(TF_ptr->eax);
		   break;
		case(GETPID_INTR):
		   TF_ptr->eax = running_pid;
		   break;
		case(STARTPROC_INTR):
		   new_pid = DeQ(&free_q);
		   StartProcISR(new_pid,TF_ptr->eax);
		   break;
		case(SEMWAIT_INTR):
		   SemWaitISR(TF_ptr->eax);
	           break;
		case(SEMPOST_INTR):
		   SemPostISR(TF_ptr->eax);
		   break; 
		case(SEMGET_INTR):
		   SemGetISR(TF_ptr->eax);
	  	   break;
		case(MSGSND_INTR):
		    MsgSndISR(TF_ptr->eax);
		    break;

		case(MSGRCV_INTR):
		    MsgRcvISR(TF_ptr->eax); 
		    break;
	    default:
		cons_printf("Panic: unknown intr ID (%d)!\n",TF_ptr->intr_id);
		breakpoint();
	}		
   
   Scheduler();  
   LoadRun(pcb[running_pid].TF_ptr); 
}
   

