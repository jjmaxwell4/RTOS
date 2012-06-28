#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "globals.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>

//globals

int main()
{	
	atomic_count = 0;
	atomic(1);
	k_init();
	
	/*Make file for kb*/
	kfid = open(kbfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755); 
		
	if(kfid<0) {
		printf("Bad Open of kbmmap file <%s> \n", kbfilename);
		exit(0);
	}
	kstatus = ftruncate(kfid, bufsize);
	
	if(kstatus) {
		printf("Failed to ftruncate the file <%s>, status = %d\n", kbfilename, kstatus);
		exit(0);
	}
	
	char childarg1[20], kchildarg2[20];
	int mypid = getpid();
	int suc1, suc2;
	
	sprintf(childarg1, "%d", mypid);
	sprintf(kchildarg2, "%d", kfid);
		
	/*Fork KB Child*/
		
	in_pid = fork();
	if(in_pid == 0)
	{
		execl("./KB", "KB", childarg1, kchildarg2, (char *)0);
		// should never reach here...
		fprintf(stderr, "iRTX: Can't exec keyboard, errno %d\n",errno);
		cleanup();
		exit(1);
	}
	sleep(.5);
	
	kmmap_ptr = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
		    bufsize,              /* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    kfid,           /* the file associated with mmap */
		    (off_t) 0);    /* Offset within a page frame */
			
    if (kmmap_ptr == MAP_FAILED){
      printf("Parent's memory map has failed, about to quit!\n");
	  die(0);  // do cleanup and terminate
    };
		
	in_mem_p = (inputbuf *) kmmap_ptr; //
	in_mem_p->flag = MEM_EMPTY;
	
	/*Now make another file for the crt*/
	
	cfid = open(crtfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755);
	if(cfid<0) {
		printf("Bad Open of crtmmap file <%s> \n", crtfilename);
		exit(0);
	}
	cstatus = ftruncate(cfid, bufsize);
	if(cstatus) {
		printf("Failed to ftruncate the file <%s>, status = %d\n", crtfilename, cstatus);
		exit(0);
	}
	
	char cchildarg2[20];
	
	sprintf(cchildarg2, "%d", cfid); 
	
	/*Fork CRT Child*/
	
	out_pid = fork();
	if(out_pid == 0)
	{
		execl("./CRT", "CRT", childarg1, cchildarg2, (char *)0);
		// should never reach here...
		fprintf(stderr, "iRTX: Can't exec CRT, errno %d\n",errno);
		cleanup();
		exit(1);
	}
	sleep(.5);
		
	cmmap_ptr = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
		    bufsize,              /* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    cfid,           /* the file associated with mmap */
		    (off_t) 0);    /* Offset within a page frame */
    if (cmmap_ptr == MAP_FAILED){
      printf("Parent's memory map has failed, about to quit!\n");
	  die(0);  // do cleanup and terminate
    };
		
	out_mem_p = (inputbuf *) cmmap_ptr;	
	out_mem_p->flag = MEM_EMPTY;
		
	atomic(0);
	
	ualarm(TIMER_INTERVAL, TIMER_INTERVAL);
	
	dispatch();
	die(1);	
}
