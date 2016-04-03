// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "isr.h"
#include "toolfunc.h"

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
}

void Producer(){
}

void PrintDriver(){
	int i, code;
	char *p;
	msg_t msg;
	msg.recipient = 2;

	printing_semaphore = SemGet(0);

	outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);
	code = inportb(LPT1_BASE+LPT_STATUS);
	for(i=0; i<50; i++)IO_DELAY();
	outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN);
	Sleep(1);
	for(;;){
		MsgRcv(&msg);
		cons_printf("PrintDriver (PID %d) now prints...\n",GetPid());
		p = msg.data;
		
		while(*p){
			outportb(LPT1_BASE+LPT_DATA, *p);
			code = inportb(LPT1_BASE+LPT_CONTROL);
			outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE);
			for(i=0; i<50; i++)IO_DELAY();
			outportb(LPT1_BASE+LPT_CONTROL, code);
			SemWait(printing_semaphore);
			
			p++;
		}
	}
}

void InitProc(){
	
	int i;
	msg_t msg;
	msg.recipient = 2;
	MyStrcpy(msg.data,"\nHello World! Team Null"); 
	while(1){
		cons_printf("0 ");
		for(i=0; i<1666668; i++)IO_DELAY();
		if(cons_kbhit()){
			char key = cons_getchar();
			switch(key)
			{
				case('p'):
					MsgSnd(&msg);
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

