// typedef.h, 159

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include "TF.h"              // trapframe type TF_t is defined here

#define TIME_LIMIT 10       // max timer count to run
#define MAX_PROC_NUM 20      // max number of processes
#define Q_LEN 20             // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes

// this is the same as constants defines: UNUSED=0, READY=1, etc.
typedef enum {FREE, READY, RUN, SLEEP, WAIT, ZOMBIE, FORKWAIT} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int runtime;              // runtime since loaded
   int total_runtime;        // total runtime since created
   int wake_time;
   TF_t *TF_ptr;             // points to trapframe of process
} pcb_t;

typedef struct {             // proc queue type
   int head, tail, len;      // where head and tail are, and current length
   int q[Q_LEN];             // indices into q[] array to place or get element
} q_t;

typedef struct {
   int limit;                //max number of processes
   q_t wait_q;               //waiting pids
} sem_t;

typedef struct {
	int sender,	//sender pid
	recipient,	//recipient pid
	OS_clock;	//sender time stamp
	char data[101];		//pass among processes for now
	int code[3];
}msg_t;

typedef struct {
	int head, tail, len;
	msg_t msg[Q_LEN];
	q_t wait_q;

}msg_q_t;

typedef struct {
	q_t TX_buffer,
	    RX_buffer,
	    echo_buffer;
	int TX_semaphore,
	    RX_semaphore,
	    echo_mode,
	    TXRDY,
	    stdin_pid,
	    stdout_pid;
} port_data_t;

typedef void (*func_ptr_t)(); // void-return function pointer type

#endif
