// isr.c, 159
#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h" 
#include "syscall.h"

void StartProcISR(int new_pid, int func_addr) {
	MyBzero( (char*) &pcb[new_pid], sizeof (pcb_t));
	pcb[new_pid].state= READY;
	
	if(new_pid > 0 ) {
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
	

	pcb[new_pid].TF_ptr->eip = (unsigned int) func_addr;
	
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
		cons_printf("\n SemWaitISR(): blocking proc %d <--",running_pid);
		running_pid = -1;
	}
	
}

void SemPostISR(int sem_id){
	
	if(sem[sem_id].wait_q.len > 0){
		int pid = DeQ(&sem[sem_id].wait_q);
		pcb[pid].state = READY;
		EnQ(pid, &ready_q);
		cons_printf("\n SemPostISR(): freeing proc %d <--",pid);
	}
	else{
		sem[sem_id].limit++;
	}
	
		
}

void MsgSndISR(int msg_addr) {
	msg_t *incoming_msg_ptr, *destination;
	int msg_q_id, freed_pid;
	incoming_msg_ptr = (msg_t *)msg_addr;
	msg_q_id = incoming_msg_ptr->recipient;
	//authenticate send and os_clock of msg
	incoming_msg_ptr->OS_clock = OS_clock;
	incoming_msg_ptr->sender = running_pid;

	if (msg_q[msg_q_id].wait_q.len <1) {
		MsgEnQ(incoming_msg_ptr, &msg_q[msg_q_id]);
	}else {
		freed_pid = DeQ(&msg_q[msg_q_id].wait_q);
		pcb[freed_pid].state= READY;
		EnQ(freed_pid, &ready_q);
		destination = (msg_t*)pcb[freed_pid].TF_ptr->eax;
		memcpy((char *) destination, incoming_msg_ptr, sizeof(msg_t));
		cons_printf("\n! MsgSndISR(): FREEING proc %d <-- !\n",freed_pid);
		
			}
}

void MsgRcvISR(int msg_addr) {
	msg_t *receiving_msg_ptr, *queued_msg_ptr;
	int msg_q_id;

	receiving_msg_ptr = (msg_t*)msg_addr;
	msg_q_id = receiving_msg_ptr->recipient;
	if (msg_q[msg_q_id].len > 0) {
		queued_msg_ptr = MsgDeQ(&msg_q[msg_q_id]);
		*receiving_msg_ptr = *queued_msg_ptr;
	
	}else {
		EnQ( running_pid, &msg_q[msg_q_id].wait_q);
		pcb[running_pid].state = WAIT;
		cons_printf("\n! MsgRcvISR(): BLOCKING proc %d <-- !\n",running_pid);
		running_pid=-1;
	}
}






