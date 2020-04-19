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
    /*
 * arg0: int [file descriptor]
 * arg1: char * [buffer to write read bytes to]
 * arg2: int [number of bytes to read]
 *
 * reads up to arg2 bytes from the current position of the file descriptor 
 * arg0 and places those bytes into arg1. The current position of the
 * file descriptor is updated by that number of bytes.
 *
 * returns number of bytes read, or -1 if there was an error.
 * If there are insufficient available bytes to complete the request,
 * reads as many as possible before returning with that number of bytes. 
 *
 * Fewer than arg2 bytes can be read in various conditions:
 * If the current position + arg2 is beyond the end of the file.
 * If this is a pipe or console device and fewer than arg2 bytes are available 
 * If this is a pipe and the other end of the pipe has been closed.
 *
 * Error conditions:
 * arg0 is not a file descriptor open for read 
 * some address between [arg1,arg1+arg2-1] is invalid
 * arg2 is not positive
 */

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


  //TODO

  int res = fileclose( fd);
  return res;
}

int sys_fstat(void) {
  // LAB1
  int fd;
  struct stat *fstat;
  
  //fd is not an open file descriptor
  if(argint(0,&fd)||argfd(0, &fd) < 0)
    return -1;

  //check if given fd is valid in the global file table
  //TODO

  //if there is an invalid address between [arg1, arg1+sizeof(fstat)]
  if(argptr(1, (char**)(&fstat), sizeof(fstat)) == -1)
    return -1;

  return filestat(fd, fstat);
}

int sys_open(void) {
  // LAB1
 /*
  * arg0: char * [path to the file]
 *  arg1: int [mode for opening the file (see inc/fcntl.h)]
 *
 * Given a pathname for a file, sys_open() returns a file descriptor, a small,
 * nonnegative integer for use in subsequent system calls. The file descriptor
 * returned by a successful call will be the lowest-numbered file descriptor
 * not currently open for the process.
 *         
 *  Each open file maintains a current position, initially zero.
 *           
 ** returns -1 on error
 *             
 *  * Errors:
 *     arg0 points to an invalid or unmapped address 
 *     there is an invalid address before the end of the string 
 *     the file does not exist
 *     since the file system is read only, flag O_CREATE is not permitted
 *     there is no available file descriptor
 *                   
 *     note that for lab1, the file system does not support file create
 *     */

  char *path; //path to the file 
  int mode; // mode got opening the file 
  int fd;

  //no available file descriptor
  //O_CREATE mode not supported for this lab 

  if(argstr(0, &path) < 0 || argint(1, &mode) < 0)
    return -1;

  //invalid or unmapped address or file dne 
  if(mode == O_CREATE) 
    return -1;

  //call appropriate file function
  fd = fileopen(path,mode);
  return fd;

}

int sys_exec(void) {
  // LAB2
  return -1;
}

int sys_pipe(void) {
  // LAB2
  return -1;
}
