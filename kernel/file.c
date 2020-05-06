
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
//Lab1 design
struct devsw devsw[NDEV];
struct file_info ftable[NFILE];
int offset =0;

//lab2 design
struct spinlock lock;


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
  

  if(iptr->type==T_DIR){ 
       unlocki(iptr);
       return -1;
  }

// trying to open a file that is not read only
  if(iptr->type==T_FILE && mode!=O_RDONLY)
      return -1;

  int foundSlot = 0;
// find open slot on process open file table pftable
 int pfd = 0;//process file descriptor index
 struct proc* p=myproc();
 for(pfd=0;pfd<NOFILE;pfd++){
    if(p->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
      foundSlot = 1;
       break;
  }
 }

 if(foundSlot == 0) {
   //   cprintf("THERE ARE NO MORE OPEN SLOTS IN PROCESS FILE TABLE\n");
   return -1;
 }


 int gfd =0;//global file descriptor index
  //Lab2 design
  for(gfd=0;gfd<NFILE;gfd++){
    if(ftable[gfd].ref == 0) { // check if slot is empty
     
      acquire(&lock);
      ftable[gfd].ref+=1;
      ftable[gfd].iptr = iptr;
      ftable[gfd].access_permission= mode;//TODO Not sure what value should be assign here
      ftable[gfd].path = path;
      //Assign pointer to pftable in slot pfd
      p->pftable[pfd] = &ftable[gfd];
      //Lab2 design
      release(&lock);
      break;

    }
 }

  //  cprintf("%s open in ftable %d and point to global ftable %d with ref %d: \n",path,pfd,gfd,ftable[gfd].ref);
  return pfd;

}

int filestat(int fd,struct stat *fstat) {
  struct proc* p = myproc();
  // not valid 
  if(p->pftable[fd] == NULL)
    return -1;

  struct file_info f = *(p->pftable[fd]);
  if(f.iptr==NULL)return -1;
  //acquire(&lock);//Not sure if I need to do it over here
  concurrent_stati(f.iptr, fstat);
  //release(&lock);
  return 0;
}

int fileclose(int fd) {

   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info f=*(p->pftable[fd]);
   if(f.iptr==NULL)return -1;
    
    acquire(&lock);
   //Decrease reference count of file by 1
   //If ref count is 1
   if(f.ref > 1) {
     p->pftable[fd]->ref -= 1;
   } else {
     // cprintf("irelease ===\n\n");
     irelease(f.iptr);
    // reset everyting
     p->pftable[fd]->iptr = 0;
     p->pftable[fd]->ref = 0;
     p->pftable[fd]->path = 0;
     p->pftable[fd]->access_permission = 0;
     p->pftable[fd]->offset=0;
    
   }
   //   cprintf("%s close  for fd =%d and ref is %d \n",f.path,fd,p->pftable[fd]->ref);

   release(&lock);
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
   
   if(!f.isPipe){
    // TODO need to change offset to ftable's struct in order to avoid multi tread issue
        offset= concurrent_readi(f.iptr,buf,f.offset,bytes_read);  
   
   	acquire(&lock);
   	p->pftable[fd]->offset+=offset;
   //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
   	release(&lock);
  	return offset;
    }else{
    //Pipe read


   } 

  }



int filewrite(int fd, char *buf, int bytes_written) { 
  
   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   
   struct file_info f=*(p->pftable[fd]);

   if(f.iptr==NULL)return -1;
   if(f.access_permission==O_RDONLY)return -1;
   if(!f.isPipe){  
   	return concurrent_writei(f.iptr, buf, f.offset, bytes_written);
   }else{
     //Pipe write

   }

}

int filedup(int fd) {
   struct proc* p = myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info f=*(p->pftable[fd]);
   if(f.iptr==NULL)return -1;
  for(int i = 0; i < NOFILE; i++) {
    if(p->pftable[i] == NULL) {
      acquire(&lock);
      p->pftable[i] = p->pftable[fd]; 
      p->pftable[fd]->ref++;     //increase reference count
      //cprintf("dup file:%s from fd = %d to new fd = %d and ref become %d \n",f.path,fd,i,p->pftable[fd]->ref);
      release(&lock);
      return i;
    }
  }
  return -1; //not available
}

void filecopy(struct proc* parent,struct proc* child){
    for(int i=0;i<NOFILE;i++){
      if(parent->pftable[i]!=NULL){
         acquire(&lock);
         child->pftable[i]=parent->pftable[i];
         child->pftable[i]->ref++;
         release(&lock);
      }
    }
}


int pipe(int *fds) {

  struct pipe *p_ptr = (struct pipe*) kalloc();
  if(p_ptr == NULL) return -1;

  int idx;
  int fd[2];

  //TODO check process file table for fds
  /*
  struct proc *p = myproc();
  int index = 0;
  for(int i=0; i<NOFILE; i++) {
    if(p->pftable[i] == NULL) {
      fds[index] = i;
      index++;
    }
    if(index == 2)
      break;
  }

//TODO handle errors
  */


  //check global file table for fds
  for(int i=0; i< NFILE; i++) {
    if(ftable[i].ref == 0 && idx < 2) { //check if slot is empty
      ftable[i].ref = 1;
      fd[idx] = i;
      idx++;
    }
  }

  //no sufficient space
  if(idx < 2) {
    if(idx == 1) //reset
      ftable[fd[0]].ref = 0;
    kfree((char*) p_ptr);
    return -1;
  }

  //arg[0] = read end of pipe
  //arg[1] = write end of pipe
  p_ptr->read_fd = fd[0];
  p_ptr->write_fd = fd[1];
  p_ptr->head = 0;
  p_ptr->tail = 0;
  p_ptr->full = false;
  p_ptr->empty = true;

  //TODO finish initializing



}
