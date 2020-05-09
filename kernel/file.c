
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

   if(!p->pftable[fd]->isPipe){
       struct file_info f=*(p->pftable[fd]);
       if(f.iptr==NULL)return -1;
   }
    acquire(&lock);
   //Decrease reference count of file by 1
   //If ref count is 1
   if(p->pftable[fd]->ref > 1) {
     p->pftable[fd]->ref -= 1;
     cprintf("close file===\n");
   } else {
     // cprintf("irelease ===\n\n");
   if(!p->pftable[fd]->isPipe){
     irelease(p->pftable[fd]->iptr);
   } // reset everyting
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
  if(!f.isPipe){
    if(f.iptr==NULL)return -1;
    if(f.access_permission==O_WRONLY)return -1;
   
    // TODO need to change offset to ftable's struct in order to avoid multi tread issue
    offset= concurrent_readi(f.iptr,buf,f.offset,bytes_read);  
   
    acquire(&lock);
    p->pftable[fd]->offset+=offset;
    //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
    release(&lock);
    return offset;
  }else{
    //Pipe read
    //Return if pipe read fd is not same 
    if(f.pipe_buffer.read_fd!=fd){
      return -1;
    }
    acquire(&p->pftable[fd]->pipe_buffer.lock);
    int size = sizeof(f.pipe_buffer.buf);
    int idx=0;
    //Need to fix the bug here
    //keep reading till the end
    cprintf("\n enter reading \n");
    while(idx<bytes_read){
      //block if empty
      while(p->pftable[fd]->pipe_buffer.empty){
	//sleep on some condition variable: right now set to middle but is wrong TODO
	cprintf("read sleep empty %d\n",p->pftable[fd]->pipe_buffer.empty);
	wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
	sleep(&p->pftable[fd]->pipe_buffer.read_fd,&p->pftable[fd]->pipe_buffer.lock);
      }
<<<<<<< HEAD
      acquire(&p->pftable[fd]->pipe_buffer.lock);
      int size = sizeof(f.pipe_buffer.buf);
      int idx=0;
      //Need to fix the bug here
      //keep reading till the end
       cprintf("\n enter reading need to read %d fd num %d\n",bytes_read, p->pftable[fd]->ref);
      while(idx<bytes_read ){
         //block if empty
         while(p->pftable[fd]->pipe_buffer.empty){
          //sleep on some condition variable: right now set to middle but is wrong TODO
           cprintf("read sleep empty %d\n",p->pftable[fd]->ref);
           wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
           sleep(&p->pftable[fd]->pipe_buffer.read_fd,&p->pftable[fd]->pipe_buffer.lock);
        }
        //Copy to buf TODO need to figure out void* and whehter to use index or just ++
      //  memmove(buf, f.pipe_buffer.buf[f.pipe_buffer.head], 1);
        buf[idx] = p->pftable[fd]->pipe_buffer.buf[p->pftable[fd]->pipe_buffer.head];
        //cprintf("r %d ",buf[idx]);
        p->pftable[fd]->pipe_buffer.head++;
        p->pftable[fd]->pipe_buffer.head%=size;
        idx++;
        p->pftable[fd]->pipe_buffer.full=false; //Everytime we succeed in reading, it won't be full
        //TODO Not sure when to call wake up
       //If head catch tail : it's empty
          // wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
       if(p->pftable[fd]->pipe_buffer.head==p->pftable[fd]->pipe_buffer.tail){
          cprintf("\nbecome emtpy ======================= \n");
          p->pftable[fd]->pipe_buffer.empty=true;
          if(p->pftable[fd]->ref<2){
           release(&p->pftable[fd]->pipe_buffer.lock);
           return bytes_read;
          }
        }       
       }
           wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
        release(&p->pftable[fd]->pipe_buffer.lock);
       cprintf("finsihing reading====\n\n");
      return bytes_read;
   } 
=======
      //Copy to buf TODO need to figure out void* and whehter to use index or just ++
      //  memmove(buf, f.pipe_buffer.buf[f.pipe_buffer.head], 1);
      cprintf("r %d ",p->pftable[fd]->pipe_buffer.empty);
      buf[idx] = p->pftable[fd]->pipe_buffer.buf[p->pftable[fd]->pipe_buffer.head];
      p->pftable[fd]->pipe_buffer.head++;
      p->pftable[fd]->pipe_buffer.head%=size;
      idx++;
      p->pftable[fd]->pipe_buffer.full=false; //Everytime we succeed in reading, it won't be full
      //TODO Not sure when to call wake up
      //If head catch tail : it's empty
      wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
      if(p->pftable[fd]->pipe_buffer.head==p->pftable[fd]->pipe_buffer.tail){
	cprintf("\nbecome emtpy =======================\n");
	p->pftable[fd]->pipe_buffer.empty=true;
      }
    }       
>>>>>>> 7b53dcccbe28af141653ca0fb6034948c0addee6

    wakeup(&p->pftable[fd]->pipe_buffer.write_fd);
    release(&p->pftable[fd]->pipe_buffer.lock);
    cprintf("finsihing reading====\n\n");
    return bytes_read;
  } 

}



int filewrite(int fd, char *buf, int bytes_written) { 
  struct proc* p =myproc();
  if(p->pftable[fd]==NULL)
    return -1;
   

  struct file_info f=*(p->pftable[fd]);
  if(!f.isPipe){  
    if(f.iptr==NULL)return -1;
    if(f.access_permission==O_RDONLY)return -1;
    return concurrent_writei(f.iptr, buf, f.offset, bytes_written);
  }else{
    // If paased in fd is not smae as pipe_write fd
    if(p->pftable[fd]->pipe_buffer.write_fd!=fd){
      return -1;
    }
    cprintf("\nenter writing  process\n");
    //Pipe write
    acquire(&p->pftable[fd]->pipe_buffer.lock);
    int idx = 0;
    int size = sizeof(f.pipe_buffer.buf);
    //Keep loop till everyting is wrote to buffer
    while(idx < bytes_written ){
      //Block if buffer is full
      while(p->pftable[fd]->pipe_buffer.full){
	//TODO need to figure out condition variable 
	cprintf("write sleep\n");
	wakeup(&p->pftable[fd]->pipe_buffer.read_fd);
	sleep(&p->pftable[fd]->pipe_buffer.write_fd, &p->pftable[fd]->pipe_buffer.lock);
      } 
      //cprintf("w %s\n",buf[idx]);
      //move memory from buf to pipe TODO need to check syntax
      //memmove(p.pipe_buffer.buf[p->pftable[fd]->pipe_buffer.tail],buf, 1);
      p->pftable[fd]->pipe_buffer.buf[p->pftable[fd]->pipe_buffer.tail] = buf[idx];
      idx++ ;
      p->pftable[fd]->pipe_buffer.tail++;
      p->pftable[fd]->pipe_buffer.tail%=size; //make it circular
      p->pftable[fd]->pipe_buffer.empty=false;
      cprintf("w %d ",p->pftable[fd]->pipe_buffer.empty);
      wakeup(&p->pftable[fd]->pipe_buffer.read_fd);
      //TODO not sure it's a right place to wakeup
      //buffer is full if tail catch head
      if(p->pftable[fd]->pipe_buffer.tail==p->pftable[fd]->pipe_buffer.head){
           
	cprintf("\n write to full====\n");
	p->pftable[fd]->pipe_buffer.full = true;
      }

    } 
    wakeup(&p->pftable[fd]->pipe_buffer.read_fd);
    release(&p->pftable[fd]->pipe_buffer.lock);
    cprintf("finsihing writing====\n\n");
    return bytes_written;
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
  
  struct proc* p = myproc();
  struct pipe *p_ptr;
  //if(p_ptr == NULL) return -1;

  int idx=0;
  // int fds[2] = {-1,-1};

  //TODO check process file table for fds
  //check global file table for fds
  int i=0;
  bool foundSlot=false; 
  for( ; i< NFILE; i++) {
    if(ftable[i].ref == 0 ) { //check if slot is empty
      ftable[i].isPipe= true;
      ftable[i].access_permission=O_RDWR;//need to check
      cprintf("file table %d is pipe",i);
      foundSlot = true;
      p_ptr = &ftable[i].pipe_buffer;
      break;
    }
  }
  //TODO handle errors
  if(!foundSlot){
    cprintf("no slot found in ftable ");
    return -1;
  }
  //check local process file table
  int j=0;
  for(;j<NOFILE;j++){
    if(p->pftable[j]==NULL && idx<2){
<<<<<<< HEAD
       fds[idx]=j;
       idx++;
       p->pftable[j] = &ftable[i];
       ftable[i].ref+=1;       
    } 
  }
 
  //no sufficient space
  if(idx < 2) {
    ftable[i].ref = 0;
    kfree((char*) p_ptr);
    cprintf("no space in process file table");
    return -1;
  }

  //arg[0] = read end of pipe
  //arg[1] = write end of pipe
  p_ptr->read_fd = fds[0];
  p_ptr->write_fd = fds[1];
  p_ptr->head = 0;
  p_ptr->tail = 0;
  p_ptr->full = false;
  p_ptr->empty = true;
  //TODO finish initializing
  return 0;
}
