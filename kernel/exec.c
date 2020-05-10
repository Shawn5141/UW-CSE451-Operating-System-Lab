#include <cdefs.h>
#include <defs.h>
#include <elf.h>
#include <memlayout.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <trap.h>
#include <x86_64.h>

int exec(int n, char *path, char **argv) {
  // your code here

  //overwrite current user's virtual address space

  //create temp vspace to use for initalization later
  struct vspace temp;
  vspaceinit(&temp);

  //read program and load it via spaceloadcode
  //load code for given progam at 'path' into vspace
  uint64_t rip; //first instruction for program
  int sz = vspaceloadcode(&temp, path, &rip); 
  if(sz == 0) {
    vspacefree(&temp);
    return -1;
  }

  //initialize stack region in user's address space
  //vspace begins at SZ_2G and grows down from here
  int res = vspaceinitstack(&temp, SZ_2G);
  if(res < 0) {
    vspacefree(&temp);
    return -1;
  }

  //address of string args on user stack
  uint64_t userStack_args[n+2];
  userStack_args[0] = 0x00; // address of pc for return - doesn't matter
  userStack_args[1 + n] = 0; //null terminator at end

  //write string args to user stack
  //create deep copy of arguments from old address space to new one via vscpacewriteova - to export data to a page table that isn't currently installed

  uint64_t va_space = SZ_2G; //vspace begins here
  for(int i=0; i< n; i++) {
    //uint64_t size = strlen(argv[i]) + 1;
    //va_space -= size;
    //    va_space = (((va_space)) & ~(7)); // round down


    uint64_t size = strlen(argv[i]) + 1 + 8; //add 1 for null termination -- add 8?
    va_space -= (size/8) *8;
 
    //write to address
    res = vspacewritetova(&temp, va_space, argv[i], size - 8); 
    if(res < 0) {
      vspacefree(&temp);
      return -1;
    }

    userStack_args[i+1] = va_space; //add arg to userStack
  }

  //go to address where we will write user stack args to user stack

  int copyLen = (n+2) * 8;
  va_space -=  copyLen;
 //    va_space = (((va_space)) & ~(7)); // round down

  //write pointers to string args in user stack
  //null term and return pc (garbage)
  if(vspacewritetova(&temp, va_space, (char*)userStack_args, copyLen) < 0) {
    vspacefree(&temp);
    return -1;
  }


  struct proc *p = myproc();
  //registers
  p->tf->rip = rip;
  p->tf->rsi = va_space + 8;
  p->tf->rdi = n;

  //ISSUE IS HERE
  p->tf->rsp = 0xffffffff801069fb;// -- dummy garbage value
 //  p->tf->rsp = va_space;



  //testing
  vspacedumpstack(&temp);

  //copy into current virtual space
  res = vspacecopy(&(myproc()->vspace), &temp);
  //free used space
  vspacefree(&temp);
  if(res < 0)
    return -1;

  //  vspacedumpstack(&(myproc()->vspace));

  //once memory is space is ready, use vspaceinstall(myproc()) to engage 
  vspaceinstall(myproc());
  return 0;
}
