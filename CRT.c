#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "globals.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>

int buf_index;

/****There is a terminate function associated with this process.***/
void out_die(int signal){
	exit(0);
}

int main (int argc, char * argv[])
{
  /**Set some variables up****/
  int parent_PCB_id, fid;
  caddr_t mmap_ptr;
  inputbuf * out_mem_p;
  char c;

  /**Set some linked memory up****/
  sigset(SIGINT,out_die);
  sscanf(argv[1], "%d", &parent_PCB_id );
  sscanf(argv[2], "%d", &cfid );
  
  cmmap_ptr =     mmap((caddr_t) 0,
             bufsize,
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             cfid,
             (off_t) 0);
  if (cmmap_ptr == MAP_FAILED){
    printf("Child memory map has failed, CRT is aborting!\n");
    out_die(0);
  }
  
  /****Set the out_mem_p ****/
  out_mem_p = (inputbuf *) cmmap_ptr;
  buf_index = 0;
  out_mem_p->flag = MEM_EMPTY;
	
   /****Now that the processes are linked the CRT process can retrieve the char 
   arrays and output them to the monitor.****/
   sleep(2);
	do {
		if(out_mem_p->flag == MEM_FULL){ //If the buffer is full of data to print
			printf("%s\n",out_mem_p->data); // Print Data
			out_mem_p->flag=MEM_EMPTY; // Reset the flag
		}
		while(out_mem_p->flag == MEM_EMPTY){
			kill (parent_PCB_id, SIGUSR2);//sends a signal to crt i_process
			usleep(100000);  //put the crt back on the processor after some time
		} 
	} while(1);
}
