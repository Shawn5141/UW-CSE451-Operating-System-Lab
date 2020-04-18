
// File descriptors
//

#include <cdefs.h>
#include <defs.h>
#include <file.h>
#include <fs.h>
#include <param.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>
#include <proc.h>
#include <fcntl.h>

struct devsw devsw[NDEV];
struct file_info ftable[NFILE];
int offset =0;
int fileopen(char *path,int mode){
  /*
 *Finds an open spot in the process open file table and has it point the global open file table entry .
 * Finds an open entry in the global open file table and allocates a new file_info struct
for the process open file table spot to point to.
Will always open a device, and only open a file if permissions passed are O_RDONLY.
Returns the index into the process open file table as the file descriptor, or -1 on failure.
 *
 * 
 */
  struct inode* iptr = namei(path); // find the inode with the path - increments reference count
  
  //need to allocate emtpy stat
  struct stat *istat;  //TODO Not sure I can create local varible here like this or I need to allocate some memory
  memset(&istat,0,sizeof(istat));
  // This function is inspired by thread on Ed : https://us.edstem.org/courses/399/discussion/28068
  if(iptr == 0)
    return -1;
  concurrent_stati(iptr,istat);
  if(iptr->type==1){ //TODO need to double check whehter it will return -1 if inode is directory //number can refer to stat.h in inc
       unlocki(iptr);
       return -1;
  }
  if(iptr->type==2 && mode!=O_RDONLY)
      return -1;

// find open slot on process open file table pftable
 int pfd = 0;//process file descriptor index
 struct proc* p=myproc();
 for(pfd=0;pfd<NOFILE;pfd++){
    if(p->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
       break;
  }
 }
 int gfd =0;//global file descriptor index
  for(gfd=0;gfd<NFILE;gfd++){
    if(ftable[gfd].iptr==NULL || ftable[gfd].iptr==iptr){//TODO Not sure how to check is empty
 //      if(ftable[gfd].iptr==iptr)cprintf("ref number %d for file %s:",ftable[gfd].ref,path);
       break;

    }
  }
  //Update ftable[gfd] file_info struct value 
  ftable[gfd].ref+=1;
  ftable[gfd].iptr = iptr;
  if(iptr==0)cprintf("why you are zero pointer");
  //ftable[gfd].offset =0;//should it be zero?
  ftable[gfd].access_permission= mode;//TODO Not sure what value should be assign here
  ftable[gfd].path = path;
  //Assign pointer to pftable in slot pfd
  p->pftable[pfd] = &ftable[gfd];

  cprintf("%s open in ftable %d and point to global ftable %d with ref %d: \n",path,pfd,gfd,ftable[gfd].ref);
  return pfd;

}

int filestat(int fd,struct stat *fstat) {
  struct proc* p = myproc();
  // not valid 
  if(p->pftable[fd] == NULL)
    return -1;

  struct file_info f = *(p->pftable[fd]);
  if(f.iptr==NULL)return -1;

  concurrent_stati(f.iptr, fstat);
  return 0;
}

int fileclose(int fd) {

   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info f=*(p->pftable[fd]);
   if(f.iptr==NULL)return -1;
   //Decrease reference count of file by 1
   //If ref count is 1
   if(f.ref > 1) {
     p->pftable[fd]->ref -= 1;
   } else {
     cprintf("irelease ===\n\n");
     irelease(f.iptr);
     p->pftable[fd]->iptr = 0;
   }
   cprintf("%s close  for fd =%d and ref is %d \n",f.path,fd,p->pftable[fd]->ref);

   //remove file from current process's file table
   p->pftable[fd] = NULL;
   return 0;
 
}

int fileread(int fd, char *buf, int bytes_read) {
   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info f=*(p->pftable[fd]);
   if(f.iptr==NULL)return -1;
   if(f.access_permission==O_WRONLY)return -1;
    
   offset= concurrent_readi(f.iptr,buf,f.offset,bytes_read);  
   
   p->pftable[fd]->offset+=offset;
   cprintf("offset right now %d and read %d bytes \n",p->pftable[fd]->offset,bytes_read);
  return offset;
}


int filewrite(int fd, char *buf, int bytes_written) { //int *bytes_written
  
   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   
   struct file_info f=*(p->pftable[fd]);

   if(f.iptr==NULL)return -1;
   if(f.access_permission==O_RDONLY)return -1;
   
   return concurrent_writei(f.iptr, buf, f.offset, bytes_written);
  
  /*
  struct proc* p = myproc();
  if(p->pftable[fd] == NULL) 
    return -1;

  struct file_info f = *(p->pftable[fd]);
  if(f.iptr == NULL)
    return -1;

  if(f.access_permission == O_RDONLY)
    return -1;

  offset = concurrent_writei(f.iptr, buf, f.offset, *bytes_written);

  p->pftable[fd]->offset += offset;

  return offset;
  */

}

int filedup(int fd) {
   struct proc* p = myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info f=*(p->pftable[fd]);
   if(f.iptr==NULL)return -1;

  for(int i = 0; i < NOFILE; i++) {
    if(p->pftable[i] == NULL) {
      p->pftable[i] = p->pftable[fd]; //TODO is this setting it to the correct value?
      p->pftable[fd]->ref++;     //TODO increase reference count
      cprintf("dup file:%s from fd = %d to new fd = %d and ref become %d \n",f.path,fd,i,p->pftable[fd]->ref);
      return i;
    }
  }
  return -1; //not available
}
