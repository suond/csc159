// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "isr.h"
#include "toolfunc.h"
#include "FileService.h"

void IdleProc() {
   
   	int i;
   
	for (;;){  	
		cons_printf("%d ", GetPid());
   	
		for(i=0; i<1666668; i++){
   			IO_DELAY();
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

void DirStr(attr_t *p, char *str) { // build str from attributes in given target
// my_msg.data has 2 parts: attr_t and target, p+1 points to target
   char *target = (char *)(p + 1);

// build str from attr_t p points to
   sprintf(str, " - - - -  size =%6d    %s\n", p->size, target);
   if ( A_ISDIR(p->mode) ) str[1] = 'D';         // mode is directory
   if ( QBIT_ON(p->mode, A_ROTH) ) str[3] = 'R'; // mode is readable
   if ( QBIT_ON(p->mode, A_WOTH) ) str[5] = 'W'; // mode is writable
   if ( QBIT_ON(p->mode, A_XOTH) ) str[7] = 'X'; // mode is executable
} 

// "dir" command, ShellProc talks to FileServicePid and port_data.stdout_pid
// make sure cmd_str ends with \0: "dir\0" or "dir obj...\0"
void DirSub(char *cmd_str, int FileServicePid) {
   char str[101];
   msg_t my_msg;
   attr_t *p;

// if cmd_str is "dir" assume "dir /\0" (on root dir)
// else, assume user specified an target after first 4 letters "dir "
   if(cmd_str[3] == ' ') {
      cmd_str += 4; // skip 1st 4 letters "dir " and get the rest: obj...
   } else {
      cmd_str[0] = '/';
      cmd_str[1] = '\0'; // null-terminate the target
   }

// apply standard "check target" protocol
   my_msg.code[0] = CHK_OBJ;
   MyStrcpy(my_msg.data, cmd_str);

   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);            // send my_msg to FileServicePid;
   MsgRcv(&my_msg);            // receive reply

   if(my_msg.code[0] != GOOD) {           // chk result code
      MyStrcpy(my_msg.data, "DirSub: CHK_OBJ return code is not GOOD!\n\0");
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);
      
      return;
   }

   p = (attr_t *)my_msg.data;     // otherwise, code[0] good, my_msg has "attr_t," 

   if(! A_ISDIR(p->mode)) {       // if it's file, "dir" it
      DirStr(p, str);             // str will be built and returned
      MyStrcpy(my_msg.data, str); // go about since p pointed to my_msg.data
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);

      return;
   }

// otherwise, it's a DIRECTORY! -- list each entry in it in loop.
// 1st request to open it, then issue reads in loop

// apply standard "open target" protocol
   my_msg.code[0] = OPEN_OBJ;
   MyStrcpy(my_msg.data, cmd_str);
   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);
   MsgRcv(&my_msg);
   
   while(1) {                     // apply standard "read obj" protocol
      my_msg.code[0] = READ_OBJ;
      my_msg.recipient = FileServicePid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);

      if(my_msg.code[0] != GOOD) break; // EOF

// do same thing to show it via STANDOUT
      p = (attr_t *)my_msg.data;
      DirStr(p, str);                // str will be built and returned
      MyStrcpy(my_msg.data, str);
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);  // show str onto terminal
      MsgRcv(&my_msg);
   }

// apply "close obj" protocol with FileServicePid
// if response is not good, display an error my_msg via port_data.stdout_pid...
   my_msg.code[0] = CLOSE_OBJ;
   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);
   MsgRcv(&my_msg);

   if(my_msg.code[0] != GOOD) {
      MyStrcpy(my_msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);
   }
}


// "cat" command, ShellProc talks to FileServicePid and port_data.stdout_pid
// make sure cmd_str ends with \0: "cat file\0"
void CatSub(char *cmd_str, int FileServicePid) {
   msg_t my_msg;
   attr_t *p;

   cmd_str += 4; // skip 1st 4 letters "cat " and get the rest

// apply standard "check target" protocol
   my_msg.code[0] = CHK_OBJ;
   MyStrcpy(my_msg.data, cmd_str);

   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);        // send my_msg to FileServicePid
   MsgRcv(&my_msg);        // receive reply

   p = (attr_t *)my_msg.data;      // otherwise, code[0] good, chk attr_t

   if(my_msg.code[0] != GOOD || A_ISDIR(p->mode) ) { // if directory
      MyStrcpy(my_msg.data, "Usage: cat [path]filename\n\0");
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);
      
      return;
   }

   // apply standard "open target" protocol
   my_msg.code[0] = OPEN_OBJ;
   MyStrcpy(my_msg.data, cmd_str);
   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);
   MsgRcv(&my_msg);
   
   while(1) {                     // apply standard "read obj" protocol
      my_msg.code[0] = READ_OBJ;
      my_msg.recipient = FileServicePid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);

      if(my_msg.code[0] != GOOD) break; // EOF

      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);  // show str onto terminal
      MsgRcv(&my_msg);
   }


   my_msg.code[0] = CLOSE_OBJ;
   my_msg.recipient = FileServicePid;
   MsgSnd(&my_msg);
   MsgRcv(&my_msg);

   if(my_msg.code[0] != GOOD) {
      MyStrcpy(my_msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
      my_msg.recipient = port_data.stdout_pid;
      MsgSnd(&my_msg);
      MsgRcv(&my_msg);
   }
}



void ShellProc(){
	msg_t my_msg;
	char login[101], password[101];
	int BAUD_RATE, divisor, my_pid, FileServicePid;

	my_pid = GetPid();
	FileServicePid = 2;
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
	MyStrcpy(my_msg.data,"\nHello World! Team Null\n\n");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
	while(1){
		while(1){
			
		
			//prompt for user name
			MyStrcpy(my_msg.data,"Team NULL login > ");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);

			//get entered user name
			my_msg.recipient = port_data.stdin_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
			MyStrcpy(login,my_msg.data);

			//prompt for password
			MyStrcpy(my_msg.data,"Team NULL password > ");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
	
			//turn off echo
			port_data.echo_mode = 0;

			//get entered password
			my_msg.recipient = port_data.stdin_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
			MyStrcpy(password,my_msg.data);
			
			//turn echo on
			port_data.echo_mode = 1;
	
			//compare user name and password
			if(MyStrlen(my_msg.data) <=0) {
				
				//prompt invalid input
				MyStrcpy(my_msg.data,"Invalid password! \n");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);
				MsgRcv(&my_msg);
			}else if(MyStrcmp(login, password, sizeof(login))) {
				MyStrcpy(my_msg.data,"Login successful, Welcome!\n");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);
				MsgRcv(&my_msg);
				break;
			}else {
				//prompt invalid input
				MyStrcpy(my_msg.data,"Invalid password! \n");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);
				MsgRcv(&my_msg);

			}
		}

		while(1){
			//prompt shell command
			MyStrcpy(my_msg.data,"Team NULL shell command > ");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);

			//get shell command
			my_msg.recipient = port_data.stdin_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
		

			if(my_msg.data[0] == '\0')
				continue;

			if(MyStrcmp("end",my_msg.data,3) || MyStrcmp("000",my_msg.data,3)) {
				MyStrcpy(my_msg.data,"Thank you for playing.  Come again!\n");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);	
				break;
			}
			if(MyStrcmp("dir",my_msg.data,3) || MyStrcmp("111",my_msg.data,3)){
				DirSub(my_msg.data, FileServicePid);
				continue;
			}

			if(MyStrcmp("cat",my_msg.data,3) || MyStrcmp("222",my_msg.data,3)){
				CatSub(my_msg.data, FileServicePid);
				continue;
			}

			//prompt shell command
			MyStrcpy(my_msg.data,"Invalid Input! Please try again.\n");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);
			MsgRcv(&my_msg);
		}
	}			
}

void InitProc(){

	while(1){
		cons_printf("0 ");
		Sleep(1);
		if(cons_kbhit()){
			char key = cons_getchar();
			switch(key)
			{
				case('b'):
					breakpoint();
					break;
				case('x'):
					exit(0);
			}	
		}
	}
}

