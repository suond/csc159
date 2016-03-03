// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"


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

void InitProc(){
	int product_sem_id, i;
	product_sem_id = SemGet(3);
	sem[product_sem_id].limit = 0;
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

