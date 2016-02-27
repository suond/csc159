// isr.c, 159
#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h" 

void StartProcISR(int new_pid) {
 //clear the PCB of the new pid
 //set its state to READY
	MyBzero( (char*) &pcb[new_pid], sizeof (pcb_t));
	pcb[new_pid].state= READY;
  //f new pid is not 0 (IdleProc),
    //then, enqueue this new pid into the ready queue
	if(new_pid !=0 ) {
	  EnQ(new_pid, &ready_q);
	}
	//build initial trapframe in proc stack
	//call mybzero() to clear the stack 1st MyBZero()
	

	//set TF_ptr of pcb to close to end (top) of stack, then fill out
	// (against last byte of stack, has space for a trapframe to build
	pcb[new_pid].TF_ptr =(TF_t*) &proc_stack[new_pid][PROC_STACK_SIZE]; 
	pcb[new_pid].TF_ptr--;	
	
	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
	//pcb[new_pid].TF_ptr
   	pcb[new_pid].TF_ptr->cs = get_cs();                     // standard fair
   	pcb[new_pid].TF_ptr->ds = get_ds();                     // standard fair
   	pcb[new_pid].TF_ptr->es = get_es();                     // standard fair
   	pcb[new_pid].TF_ptr->fs = get_fs();                     // standard fair
   	pcb[new_pid].TF_ptr->gs = get_gs();                     // standard fair


	if (new_pid == 0) {
		pcb[new_pid].TF_ptr->eip = (unsigned int)IdleProc;
	}else {
		pcb[new_pid].TF_ptr->eip = (unsigned int)UserProc;
	}
}

void EndProcISR() {
   //if running PID is 0 (IdleProc should not let exit),
    //  then, just return;
      if (running_pid == 0) {
		return;
	}
// change state of running process to FREE
// queue the running PID to free queue
// set running PID to -1 (now none)
	pcb[running_pid].state = FREE;
	EnQ(running_pid, &free_q);
	running_pid = -1;
	return;

}        

void TimerISR() {
 //just return if running PID is -1 (not any valid PID)
 //(shouldn't happen, a Panic message can be considered)
	if (running_pid == -1){
		cons_printf("error, running pid should not be -1 \n");
		return;
	}
// in PCB, upcount both runtime and total_runtime of running process
	pcb[running_pid].runtime+=1;
	pcb[running_pid].total_runtime+=1;
/*f the runtime has reached TIME_LIMIT:
      reset its runtime
      change its state to READY
      queue it to ready queue
      set running PID to -1
      (Scheduler() will get next PID from ready queue if any;
      if none, Scheduler will pick 0 as running PID)
*/
	if (pcb[running_pid].total_runtime == TIME_LIMIT) {
		pcb[running_pid].total_runtime = 0;
		pcb[running_pid].state = READY;
		EnQ(running_pid, &ready_q);
		running_pid = -1;
		
	}
}
void GetPidISR(){
	pcb[running_pid].TF_ptr->eax = running_pid;
}
void Sleep_ISR(int sleep_secs) {
    int wake_time;
    wake_time = (sleep_secs*100) + OS_clock;
    pcb[running_pid].wake_time = wake_time;
    EnQ(running_pid, &sleep_q);
    pcb[running_pid].state = SLEEP;
    running_pid = -1;

}



