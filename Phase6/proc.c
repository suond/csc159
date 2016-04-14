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
			SemWait(printing_semaphore);
			outportb(LPT1_BASE+LPT_DATA, *p);
			code = inportb(LPT1_BASE+LPT_CONTROL);
			outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE);
			for(i=0; i<50; i++)IO_DELAY();
			outportb(LPT1_BASE+LPT_CONTROL, code);
			
			p++;
		}
	}
}

void StdinProc(){
	msg_t my_msg;
	char *p;
	char ch;
	int my_pid = GetPid();

	for(;;){
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);
		p = my_msg.data;
		
		while(1){
			SemWait(port_data.RX_semaphore);
			ch = DeQ(&port_data.RX_buffer);
			if(ch == '\r') break;
			*p++ = ch;
		}
		*p = '\0';
		my_msg.recipient = my_msg.sender;
		MsgSnd(&my_msg);
	}
}

void StdoutProc(){
	msg_t my_msg;
	char *p;
	int my_pid;
	my_pid = GetPid();
	
	for(;;){
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);
		p = my_msg.data;
		
		while(*p){
			SemWait(port_data.TX_semaphore);
			EnQ(*p,&port_data.TX_buffer);
			if(*p == '\n'){
				SemWait(port_data.TX_semaphore);
				EnQ('\r',&port_data.TX_buffer);
			}
			TipIRQ3();
			p++;
		}
		my_msg.recipient = my_msg.sender;
		MsgSnd(&my_msg);
	}
}

void ShellProc(){
	msg_t my_msg;
	char a_string[101];
	int BAUD_RATE, divisor, my_pid;

	my_pid = GetPid();
	MyBzero((char*)&port_data, sizeof(port_data_t));
	port_data.TX_semaphore = SemGet(Q_LEN);
	port_data.RX_semaphore = SemGet(0);
	port_data.echo_mode = 1;
	port_data.TXRDY = 1;
	port_data.stdin_pid = my_pid+1;
	port_data.stdout_pid = my_pid+2;
	
	BAUD_RATE = 9600;              // Mr. Baud invented this
   	divisor = 115200 / BAUD_RATE;  // time period of each baud
  	 outportb(COM2_IOBASE+CFCR, CFCR_DLAB);          // CFCR_DLAB 0x80
  	 outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
  	 outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
   // B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
  	 outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
  	 outportb(COM2_IOBASE+IER, 0);
   // C. raise DTR, RTS of the serial port to start read/write
   	outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   	IO_DELAY();
   	outportb(COM2_IOBASE+IER, IER_ERXRDY|IER_ETXRDY); // enable TX, RX events
   	IO_DELAY();

	MyStrcpy(my_msg.data,"\n\n\nHello World! Team Null\n"); 
	my_msg.recipient = port_data.stdout_pid;
	MsgSnd(&my_msg);
	my_msg.recipient = my_pid;
	MsgRcv(&my_msg);
	for(;;){
		MyBzero((char*)&my_msg,sizeof(msg_t));
		//prompt
		MyStrcpy(my_msg.data,"\nEnter a command: ");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);

		//get entered
		my_msg.recipient = port_data.stdin_pid;
		MsgSnd(&my_msg);
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);
		MyStrcpy(a_string,my_msg.data);

		//prompt just entered
		MyStrcpy(my_msg.data,"Just Entered-> ");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);

		//show a_string on terminal
		MyStrcpy(my_msg.data,a_string);
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		my_msg.recipient = my_pid;
		MsgRcv(&my_msg);
	}
}

void InitProc(){
	
	int i;
	msg_t msg;
	msg.recipient = 2;
	MyStrcpy(msg.data,"Hello World! Team Null\n"); 
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

