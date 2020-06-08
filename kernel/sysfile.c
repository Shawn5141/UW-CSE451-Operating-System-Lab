//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include <cdefs.h>
#include <defs.h>
#include <fcntl.h>
#include <file.h>
#include <fs.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>

struct file_info ftable[NFILE];
int sys_dup(void) {
  // LAB1
  int fd;

  if(argint(0,&fd)<0 || argfd(0, &fd) < 0)
    return -1;

  return filedup(fd);

}

int sys_read(void) {
  // LAB1
  int fd;
  char *buf;
  int bytes_read;
  //fd is not a file descriptor open for read
  if(argint(0,&fd)<0||argfd(0, &fd) < 0)
    return -1;


  //Number of bytes to read is not positive
  argint(2, &bytes_read);
  if(bytes_read < 0)
    return -1;

  //Some address between [arg1, arg1+arg2-1] is invalid
  if(argptr(1, &buf, bytes_read) < 0)
    return -1;

  bytes_read=fileread(fd, buf, bytes_read);

  return bytes_read;
}

int sys_write(void) {
  /*
  //you have to change the code in this function.
 ` // Currently it supports printint one character to the screen.

  int n;
  char *p;

  if(argint(2, &n) < 0 || argptr(1, &p, n) < 0 )
    return -1;
  uartputc((int)(*p));
  return 1;
  */

  int bytes_written;
  char *buf;
  int fd;
  //argint : get file descriptor from argument 
  //argfd : check this fd is valid (not exceeding the range)
  //argint : get byte_written from arguemnt
  //argptr : get char need to be written from argument and put in buf
  //argstr : read buff and check it's positive
  if(argint(0,&fd)<0||argfd(0,&fd)<0|| argint(2, &bytes_written) < 0|| 
     argptr(1, &buf, bytes_written) < 0||  argstr(1,&buf)<0)
    return -1;
 // uartputc((int)(*buf));//print it out on console;
  return  filewrite(fd, buf, bytes_written);
  
}

int sys_close(void) {
  // LAB1
  int fd;

  argint(0,&fd);
  //fd is not an open file descriptor
  //check if given fd is valid in global file table

  if(argfd(0, &fd) < 0)
    return -1;

  //  if(ftable[fd].ref == 0)
  // return -1;

  int res = fileclose(fd);
  //  cprintf("THE RES FROM FILECLOSE IS %d\n", res);
  return res;
}

int sys_fstat(void) {
  // LAB1
  int fd;
  struct stat *fstat;
  
  //fd is not an open file descriptor
  if(argint(0,&fd)||argfd(0, &fd) < 0)
    return -1;


  //if there is an invalid address between [arg1, arg1+sizeof(fstat)]
  if(argptr(1, (char**)(&fstat), sizeof(fstat)) == -1)
    return -1;

  return filestat(fd, fstat);
}

int sys_open(void) {
  // LAB1

  char *path; //path to the file 
  int mode; // mode got opening the file 
  int fd;

  if(argstr(0, &path) < 0 || argint(1, &mode) < 0)
    return -1;

  //O_CREATE mode not supported for this lab 
  //if(mode == O_CREATE) 
  //  return -1;
  
  struct inode *ptr = namei(path);
   if (mode != O_RDONLY && mode != O_WRONLY && mode != O_RDWR && mode != O_CREATE && 
      (mode != (O_CREATE | O_RDWR))) {
    return -1;
  }
  if ((mode & 0xF00) == O_CREATE && ptr == NULL) {
    cprintf("sys_open file creattion\n");
    ptr =filecreate(path);
  }

  //call appropriate file function
    //acquire(&ftable.lock);
  fd = fileopen(path,mode);
   // release(&ftable.lock);
  return fd;

}

int sys_exec(void) {
  // LAB2
  
  char *path; //path to executable file
  char *args[MAXARG]; // array of strings for arguments - 1-D array of pointers to char - max constant

  // arg0 points to invalid or unmapped address
  if(argstr(0, &path) < 0)
     return -1;

  if(argptr(0, &path, strlen(path)) < 0)
    return -1;
 
  int address;
  if(argint(1, &address) < 0)
    return -1;

  //Cycle through until null argument found
  for(int i=0; i< MAXARG; i++) {

    //get address of each argument
    int aa;
    if(fetchint(address + 8*i, &aa) < 0) //if invalid
      return -1;

    //get argument using its address
    if(fetchstr(aa, &args[i]) < 0)
      return -1;

    if(args[i] == '\0') {
       return exec(i, path, args);
    }
  } // end for loop

  return -1; //error 
}

int sys_pipe(void) {
  // LAB2
  int *pipe_fds; //pointer to an array of two file descriptors

  //Create a pipe and two open file descriptors
  
  if(argptr(0, (char**) &pipe_fds, sizeof(int) *2) < 0){
    return -1;
  }


  int res = pipe(pipe_fds);

  return res;
}
