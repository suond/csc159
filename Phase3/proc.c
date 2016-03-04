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
	int sleep_secs = (GetPid()%5)+1; // determine its sleep seconds (1-5) according to its PID 
	int i;
	while(1){
		Sleep(sleep_secs);
		SemWait(product_sem_id);
		if (product_count == 0)
		{
			cons_printf( "Consumer %d wants to consume but no product...\n", GetPid());
		}else{
			cons_printf("Consumer %d is consuming...\n", GetPid());
			product_count -= 1;
			for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
			cons_printf("Product count is now %d\n", product_count);

		}
		SemPost(product_sem_id);
	}
}

void Producer(){
	//int sleep_secs = (GetPid()%5)+1; // determine its sleep seconds (1-5) according to its PID 
	int i;
	cons_printf("testP");
	while(1){
		SemWait(product_sem_id);
		for(i=0;i<1666000;i++) IO_DELAY(); // delay for about 1 sec
		product_count += 1;
		cons_printf("Product count is now %d\n", product_count);
		SemPost(product_sem_id);
		//Sleep(sleep_secs);
	}
	

}

void InitProc(){
	
	int i;
	product_sem_id = SemGet(1);
	product_count = 0;
	while(1){
		for(i=0; i<1666668; i++)IO_DELAY();
		if(cons_kbhit()){
			char key = cons_getchar();
			switch(key)
			{
				case('p'):
					cons_printf("test1");
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

