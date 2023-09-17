#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/mman.h>
#include <sys/wait.h>
#include <stddef.h>
#define errExit(msg)        \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)


size_t page_size;
size_t page_no;
size_t size = 4096;

pthread_t thr;
int thr_status;
int fd;
static void* fault_handler_thread(void* arg) {



   long uffd;
    struct uffdio_writeprotect uffdio_wr; 
    struct uffdio_continue con;

  struct timeval ts;
  struct timeval te;
  double start;
  double end;


     /* userfaultfd file descriptor */
    uffd = (long) arg;
    static struct uffd_msg msg;
    ssize_t             nread;

    /* Loop, handling incoming events on the userfaultfd
       file descriptor. */

    for (;;) {
	    /* See what poll() tells us about the userfaultfd. */
	    struct pollfd pollfd;
	    int nready;
	    pollfd.fd = uffd;
	    pollfd.events = POLLIN;
	    nready = poll(&pollfd, 1, -1);
	    if (nready == -1)
		    errExit("poll");

		printf("poll() returns: nready = %d; "
		      "POLLIN = %d; POLLERR = %d\n",
			    nready, (pollfd.revents & POLLIN) != 0,
			    (pollfd.revents & POLLERR) != 0);
	    nread = read(uffd, &msg, sizeof(msg));
	    if (nread == 0) {
		    printf("EOF on userfaultfd!\n");
		    exit(EXIT_FAILURE);
	    }

	    if (nread == -1)
		  errExit("nread failure\n");

	    printf("flags = %"PRIx64"; ", msg.arg.pagefault.flags);


	     if((msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WP)){	

            //start time

		     gettimeofday (&ts,NULL);
		     start =
			     (ts.tv_sec) * 1000000 + (ts.tv_usec) ; // convert tv_sec & tv_usec to millisecond

		     uffdio_wr.mode = !UFFDIO_WRITEPROTECT_MODE_WP;
		     uffdio_wr.range.start  = (uintptr_t) msg.arg.pagefault.address;
		     uffdio_wr.range.len = page_size;
		     void *addr = (uintptr_t *) msg.arg.pagefault.address;


		     int err =  munmap(addr, page_size);
		     if(err == -1) errExit("unmap failed");

		     addr = mmap(addr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd , 0);
		     if (addr == MAP_FAILED)
			     errExit("remap success");

		     struct uffdio_range uffdio_wake;
		     uffdio_wake.start = (uintptr_t)addr;
		     uffdio_wake.len = page_size;

		     if (ioctl(uffd, UFFDIO_WAKE, &uffdio_wake) == -1)
			     errExit("ioctl-UFFDIO_continue");


		     gettimeofday(&te,NULL);
		     end =
			     (te.tv_sec) * 1000000 + (te.tv_usec) ; // convert tv_sec & tv_usec to millisecond




		     printf("start:%lf\tEnd:%lf\tDuration:%lf\n", start,end,end-start);

		     //end time

		     /*  if (ioctl(uffd, UFFDIO_WRITEPROTECT, &uffdio_wr) == -1)
			 errExit("ioctl-UFFDIO_WRITE");*/


		     /*struct uffdio_continue cont;

		       cont.range.start = (uintptr_t) addr;
		       cont.range.len = page_size;
		       cont.mode = 0;*/

		     /*if (ioctl(uffd, UFFDIO_CONTINUE, &cont) == -1)
		       errExit("ioctl-UFFDIO_continue");*/

		     /*  if (ioctl(uffd, UFFDIO_WRITEPROTECT, &uffdio_wr) == -1)
			 errExit("ioctl-UFFDIO_WRITE");*/




	     }


    }




    /*struct uffdio_copy cpy;
      cpy.dst = (uintptr_t)msg.arg.pagefault.address & ~(page_size-1);
      cpy.src = (uintptr_t)page;
      cpy.len = 4096;
      cpy.mode = 0;


      int ret = ioctl(uffd, UFFDIO_COPY, &cpy);
      if (ret == -1) {
      errExit("Failed UFFDIO_COPY in  with errno");
      return ret;
      }*/
}




static long register_userfault (char *addr, size_t len){


	struct uffdio_api uffdio_api;
	struct uffdio_register uffdio_register;
	struct uffdio_writeprotect uffdio_wp;
	int s;
	long uffd;

	//int *addr = (int *)vaddr;

	for(size_t i=0; i<page_no; i++)
		addr[i*size]=0;

	//creating userfault fd objects
	uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd == -1)
		errExit("userfaultfd");

	uffdio_api.api = UFFD_API;
	uffdio_api.features = UFFD_FEATURE_PAGEFAULT_FLAG_WP;
	if (ioctl(uffd, UFFDIO_API, &uffdio_api) == -1)
		errExit("ioctl-UFFDIO_API");



	//Under a single mode multiple operations might be possitble, taht's why ned to register a new ioctl for writeprotect;

	uffdio_register.range.start = (uintptr_t) addr;
	uffdio_register.range.len = len;
	uffdio_register.mode =  UFFDIO_REGISTER_MODE_WP;
	if (ioctl(uffd, UFFDIO_REGISTER, &uffdio_register) == -1)
		errExit("ioctl-UFFDIO_REGISTER");

	//ioctl writeprotect
	uffdio_wp.range.start = (uintptr_t)addr;
	uffdio_wp.range.len = len;
	uffdio_wp.mode = UFFDIO_WRITEPROTECT_MODE_WP;
	if (ioctl(uffd, UFFDIO_WRITEPROTECT, &uffdio_wp) == -1)
		errExit("ioctl-UFFDIO_WRITEPROTECT");

	return uffd;


}


/*
   int main()

   {
   size_t len = page_size;
   long uffd;
   char *addr;
   pthread_t thr;
   int s;
   addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE| MAP_ANONYMOUS, -1 , 0);
   if (addr == MAP_FAILED)
   errExit("mmap");

   uffd = register_userfault(addr, len);

   s = pthread_create(&thr, NULL, fault_handler_thread, (void*) uffd);
   if (s != 0) {
   errno = s;
   errExit("pthread_create");
   }

   for(int i=0; i<4096;i++)
   addr[i]=i;


   exit(EXIT_SUCCESS);
   }*/
