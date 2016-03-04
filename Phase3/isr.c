// isr.c, 159
#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h" 

void StartProcISR(int new_pid, int func_addr) {
	MyBzero( (char*) &pcb[new_pid], sizeof (pcb_t));
	pcb[new_pid].state= READY;
	
	if(new_pid !=0 ) {
	  EnQ(new_pid, &ready_q);
	}
	MyBzero( (char*) &proc_stack[new_pid], PROC_STACK_SIZE);
	pcb[new_pid].TF_ptr =(TF_t*) &proc_stack[new_pid][PROC_STACK_SIZE]; 
	pcb[new_pid].TF_ptr--;	
	
	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
   	pcb[new_pid].TF_ptr->cs = get_cs();                     // standard fair
   	pcb[new_pid].TF_ptr->ds = get_ds();                     // standard fair
   	pcb[new_pid].TF_ptr->es = get_es();                     // standard fair
   	pcb[new_pid].TF_ptr->fs = get_fs();                     // standard fair
   	pcb[new_pid].TF_ptr->gs = get_gs();                     // standard fair
	

	pcb[new_pid].TF_ptr->eip = (unsigned int)func_addr;
	
}

void TimerISR() {
 
	if (running_pid == -1){
		cons_printf("error, running pid should not be -1 \n");
		return;
	}
	pcb[running_pid].runtime+=1;
	pcb[running_pid].total_runtime+=1;

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

int SemGetISR(int limit){
	//alloc semid from sem_q
	// if -1
		//put -1 into ebx of tf
		//return -1
	//clear sem[semId]
	//set sem[semId]
	//limit = limit
	//return sem_id
	int sem_id;
	sem_id = DeQ(&sem_q);
	if(sem_id == -1){
		pcb[running_pid].TF_ptr->ebx= -1;
		return -1;
        }
	MyBzero((char*)&sem[sem_id].wait_q,sizeof(q_t));
	sem[sem_id].limit = limit;
	pcb[running_pid].TF_ptr->ebx= sem_id;
	return sem_id;
}

void SemWaitISR(int sem_id){
	
	if(sem[sem_id].limit > 0){
		sem[sem_id].limit--;
		return;
	}
	else{
		pcb[running_pid].state = WAIT;
		EnQ(running_pid, &sem[sem_id].wait_q);
		running_pid = -1;
	}
	
}

void SemPostISR(int sem_id){
	
	if(sem[sem_id].wait_q.len > 0){
		int pid = DeQ(&sem[sem_id].wait_q);
		pcb[pid].state = READY;
		EnQ(pid, &ready_q);
	}
	else{
		sem[sem_id].limit++;
	}
	
		
}







