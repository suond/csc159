// toolfunc.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int byte_size) {
	int i;
	for(i=0; i<byte_size; i++){
		*p=0;
		p++;		
	}	
}

void EnQ(int pid, q_t *p) {
	if(p->len == Q_LEN){
		cons_printf("Queue is full.\n");
		return;
	}
	p->len++;
	p->q[p->tail] = pid;
	p->tail = (p->tail+1)%(Q_LEN);	
}

int DeQ(q_t *p) { // return -1 if q is empty
	int pid;
	if(p->len == 0) {
		return -1;
	}
	
	pid = p->q[p->head];
	p->head = (p->head+1)%(Q_LEN);
	p->len--;
	return pid;

}
void checkWait() {
	int temp_pid;
	q_t temp_q;	
	temp_pid = DeQ(&sleep_q);
	MyBzero((char *) &temp_q, sizeof (q_t) );
		while (temp_pid !=-1){
			if (OS_clock == pcb[temp_pid].wake_time) {
				EnQ(temp_pid, &ready_q);
				pcb[temp_pid].state = READY;
				temp_pid = DeQ(&sleep_q);
			}else {
				EnQ(temp_pid, &temp_q);
				temp_pid = DeQ(&sleep_q);
			}
		}
		temp_pid = DeQ(&temp_q);
		while (temp_pid != -1) {
			EnQ(temp_pid, &sleep_q);
			temp_pid = DeQ(&temp_q);
		} 
}

void MsgEnQ(msg_t *msg_ptr, msg_q_t *msg_q_ptr) {
	if (msg_q_ptr->len == Q_LEN) {
		cons_printf("Queue is full.\n");
		return;
	}
	msg_q_ptr->msg[msg_q_ptr->tail] = *msg_ptr;
	msg_q_ptr->tail = (msg_q_ptr->tail+1)%(Q_LEN);
	msg_q_ptr->len++;		
	
}
msg_t *MsgDeQ(msg_q_t *msg_q_ptr) {
	msg_t *returnvalue;      
	if (msg_q_ptr->len <1)  {
		cons_printf("Queue is empty!");      		
		return 0;
	}
	returnvalue = &(msg_q_ptr->msg[msg_q_ptr->head]);
	msg_q_ptr->head = (msg_q_ptr->head+1)%(Q_LEN);
	msg_q_ptr->len--;
      
    return returnvalue;
}

void MyStrcpy(char * dest, char *src) {
	while (*src != '\0') *dest++ = *src++;
	*dest = '\0';	
}

void MyMemcpy(char *dest, char *src, int size){
	while(size--) *dest++ = *src++;
}

int MyStrcmp(char *p, char *q, int size){
	while(size--){
		if( *p++ != *q++)
			return 0;
	}
	return 1;
}

int MyStrlen(char *str){
	int len = 0;
	while(*str !='\0'){
		len++;
		str++;
	}
	return len;
}
