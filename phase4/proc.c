// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "isr.h"


void IdleProc() {
   
   	int i;
   
	for (;;){  	
		cons_printf("%d ", GetPid());
   	
		for(i=0; i<1666668; i++){
   			IO_DELAY();
   		}
   	}
}

void Consumer(){
	
	int i,my_pid;
	msg_t my_msg;
	my_pid = GetPid();
	while(1){
		my_msg.recipient = 0;
		MsgRcv(&my_msg);
		cons_printf("\n-- Consumer (%d) consuming data %d...\n", my_pid,my_msg.data);
		for(i=0;i<3333333;i++) IO_DELAY(); // delay for about 3 sec
		
	}
}

void Producer(){
	
	int i, my_pid;
	static int count;
	msg_t my_msg;
	my_pid = GetPid();
	while(1){
		my_msg.recipient = 0;
		my_msg.data = my_pid*100+ count++;		
		cons_printf("\n++ Producer (%d) producing data %d...\n",my_pid, my_msg.data);
		for(i=0;i<3333333;i++) IO_DELAY(); // delay for about 2 sec
		MsgSnd(&my_msg);
		
	}
	

}

void InitProc(){
	
	int i;
	while(1){
		for(i=0; i<1666668; i++)IO_DELAY();
		if(cons_kbhit()){
			char key = cons_getchar();
			switch(key)
			{
				case('p'):
					StartProc(Producer);
					break;
				case('c'):
					
					StartProc(Consumer);
					break;
				case('b'):
					breakpoint();
					break;
				case('x'):
					exit(0);
			}	
		}
	}
}

