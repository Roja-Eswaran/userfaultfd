#include "./write_protect.h"

int main(){

  //int N=5;

  long uffd;
  
  printf("Enter the page numbers:\n");
   scanf("%ld", &page_no);




   page_size = page_no * size;

   fd = open("/mnt/test_file/memory", O_CREAT | O_RDWR, 0644);
   if(fd == -1)	perror("Issue opening bf");


   printf("Page size:%ld\n", page_size);
//  printf("FD:%d\n", fd);

 /* lseek(fd, 0, SEEK_SET);
  if (write(fd, empty_mem, page_size) == -1) perror("Write Error");
  lseek(fd, 0, SEEK_SET);*/





  char *ptr = mmap ( NULL, page_size, 
      PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ); //anon

  if(ptr == MAP_FAILED){
    printf("Mapping Failed\n");
    return 1;
  }

  uffd =  register_userfault(ptr, page_size);
  thr_status = pthread_create(&thr, NULL, fault_handler_thread, (void*) uffd);
        if (thr_status != 0) {
                errno = thr_status;
                errExit("pthread_create");
        }





 for(size_t i=0; i<page_no; i++){
      char val = ptr[i*size];
//      printf("Val:%c\n", val);
      ptr[(i*size)+2] = 0;
 }

/*  printf("The elements of the array => ");
  for(int i=0; i<page_no; i++)
     printf("[%c] ",ptr[i*size]);

  printf("\n");*/


  /*int err = munmap(ptr, 10*sizeof(int));
  if(err != 0){
     printf("UnMapping Failed\n");
     return 1;
  }*/

  return 0;
}
