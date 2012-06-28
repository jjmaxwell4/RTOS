#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include "globals.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>

int buf_index;

void in_die(int signal) {
     exit(0);
}

int main (int argc, char * argv[])
{
	/*Set up some Variables*/
	int parent_PCB_id = argc;
	int fid;
	caddr_t mmap_ptr;
	inputbuf *in_mem_p;
	char c[MAX_CHAR];

	// If parent tells us to terminate, then clean up first
	sigset(SIGINT,in_die);
	
	//Gets id of parents (to signal) and the file id of mmap.
	sscanf(argv[1], "%d", &parent_PCB_id);
	sscanf(argv[2], "%d", &kfid );

	kmmap_ptr =     
		mmap((caddr_t) 0,
		bufsize,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		kfid,
		(off_t) 0);
  
	if (kmmap_ptr == MAP_FAILED){
		printf("Child memory map has failed, KB is aborting! Error <%s> ...\n", strerror(errno));
		in_die(0);
	}
	in_mem_p = (inputbuf *) kmmap_ptr;
	buf_index = 0;
	in_mem_p->flag = MEM_EMPTY; // Set the flag to “Buffer Empty”

	do { 
		fgets(c, MAX_CHAR, stdin);  // This only returns when <cr> is pressed
		strcpy(in_mem_p->data, c); // Read in data from buffer
		in_mem_p->flag = MEM_FULL;  // Set the flag
		kill(parent_PCB_id ,SIGUSR1);  // Signal the parent
		buf_index = 0;
		while(in_mem_p->flag == MEM_FULL){
			usleep(100000); // Sleep until the RTX has read the data
		}
	} while(1); // Loops Forever
}
