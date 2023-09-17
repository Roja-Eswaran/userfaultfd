#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
//#define size 4096
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
int main(){

 

  int fd = open("/mnt/test_file/memory", O_CREAT | O_RDWR, 0644);
  if(fd == -1)	perror("Issue opening bf");
  size_t size = 4096;
  size_t page_no;
  size_t page_size;


  printf("Enter the page numbers:\n");
  scanf("%ld", &page_no);

  page_size = page_no * size;

  printf("Page size:%ld\n", page_size);

  lseek(fd, 0, SEEK_SET);
  
  char *empty_mem = (char *)malloc(sizeof(char)*size);
   memset(empty_mem,0,size);


  for(size_t index = 0; index<page_no; index++){
           lseek(fd, (index*size), SEEK_SET);
  	if (write(fd, empty_mem, size) == -1) perror("Write Error");
	   
   }



   char *ptr = mmap ( NULL, page_size, 
      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

  printf("Fd:%d\n", fd);
  if(ptr == MAP_FAILED){
    printf("Mapping Failed\n");
    return 1;
  }

  // Fill the elements of the array
  size_t index = 0;

  for(int i='a'; i<='z' && index<page_no; i++){
     
     *(ptr+(index*size)) = i;
     index ++;

    if( i == 'z')
	i = 'a'-1;
 }

  int err = munmap(ptr, page_size);
  if(err != 0){
     printf("UnMapping Failed\n");
     return 1;
  }

  return 0;
}
