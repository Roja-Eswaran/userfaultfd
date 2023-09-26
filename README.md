# userfaultfd-mmap
This code measures the time taken to mmap a region of memory through userfaultfd ioctl.
Make sure the linux kernel is >= 5.15.131
1) Set up the tmpfs file and execute mmap1 which sets up the shared memory region which will then be used by mmap2 program to replace its anonymous memory.
2) Execute mmap1 - Give number of pages to be mapped
3) Execute mmap2 - Give the same number of pages as the input. Now, the code will return the total time in microseconds needed to replace the anonymous memory region with the shared mmap from mmap1

Please apply the patches inorder to use shared memory instead of anonymous page. Kernel version should be >=5.19 to support this feature. 
