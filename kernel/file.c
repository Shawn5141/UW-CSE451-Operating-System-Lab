
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
  //  cprintf("ENTERED FILEOPEN\n");
  struct inode* iptr = namei(path); // find the inode with the path - increments reference count
  
//need to allocate emtpy stat
  struct stat istat ;  //TODO Not sure I can create local varible here like this or I need to allocate some memory
  //memset(&istat,0,sizeof(istat));
  // This function is inspired by thread on Ed : https://us.edstem.org/courses/399/discussion/28068
  if(iptr == 0)
    return -1;

  concurrent_stati(iptr,&istat);

  /*
     
  if(iptr->type==T_DIR){ 
    cprintf("TRYING TO OPEN DIRECTORY\n");
  //       unlocki(iptr);
    // cprintf("TRY TO UNLOCK PTR\n");
       return -1;
  }
  */

  if(mode == T_DEV) // The only type cannot open should be T_DEV
    return -1; 
  // trying to open a file that is not read only
  if(iptr->type==T_FILE && mode!=O_RDONLY)
      return -1;


  int foundSlot = 0;
  // find open slot on process open file table pftable
  int pfd = 0;//process file descriptor index
  struct proc* p=myproc();
  for(pfd=0;pfd<NOFILE;pfd++){
    if(p->pftable[pfd]==NULL) { 
      foundSlot = 1;
      break;
    }
  }

  if(foundSlot == 0) {
   return -1;
  }

  int gfd =0;//global file descriptor index
  //Lab2 design
  for(gfd=0;gfd<NFILE;gfd++){
    if(ftable[gfd].ref == 0) { // check if slot is empty
      
      acquire(&lock);
      ftable[gfd].ref=1;
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
  // cprintf("%s open in ftable %d and point to global ftable %d with ref %d: \n",path,pfd,gfd,ftable[gfd].ref);
  return pfd;  
}

int filestat(int fd,struct stat *fstat) {
  struct proc* p = myproc();
  // not valid 
  if(p->pftable[fd] == NULL)
    return -1;

  struct file_info f = *(p->pftable[fd]);
  if(f.isPipe == false && f.iptr==NULL)
    return -1;  

  //acquire(&lock);//Not sure if I need to do it over here
  concurrent_stati(f.iptr, fstat);
  //release(&lock);
  return 0;
}

int fileclose(int fd) {
  //  cprintf("ENTERED CLOSE\n");
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
     p->pftable[fd]->ref--;
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
     //     cprintf("close  for fd =%d and ref is %d \n",fd,p->pftable[fd]->ref);

   release(&lock);
   //remove file from current process's file table
   p->pftable[fd] = NULL;
   return 0;
}

int fileread(int fd, char *buf, int bytes_read) {
   struct proc* p =myproc();
   if(p->pftable[fd]==NULL)return -1;
   struct file_info *f=p->pftable[fd];

   if(!f->isPipe){
     if(f->iptr==NULL)return -1;
     if(f->access_permission==O_WRONLY)return -1;
     
     // TODO need to change offset to ftable's struct in order to avoid multi tread issue
     offset= concurrent_readi(f->iptr,buf,f->offset,bytes_read);  
     
     acquire(&lock);
     p->pftable[fd]->offset+=offset;
     //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
     release(&lock);
  	return offset;	
	
   }else{
     //Pipe read
     //Return if pipe read fd is not same 
     if(f->pipe_buffer.read_fd!=fd){
       return -1;
     }
     acquire(&f->pipe_buffer.lock);
     int size = sizeof(f->pipe_buffer.buf);
     int res=0;
     //Need to fix the bug here
     //keep reading till the end
     while(f->pipe_buffer.head==f->pipe_buffer.tail){
       if(myproc()->killed !=0 ) {
	 release(&f->pipe_buffer.lock);
	 return -1;
       }
       if(f->ref==1){
	 release(&f->pipe_buffer.lock);
	 return 0;
       } 
       wakeup(&f->pipe_buffer.write_fd);
       sleep(&f->pipe_buffer.read_fd,&f->pipe_buffer.lock);
     }
     
     for(int i=0;i<bytes_read;i++){
       if(myproc()->killed !=0) return -1;
       if(f->pipe_buffer.head==f->pipe_buffer.tail){
	 break;
       }
       buf[i] = f->pipe_buffer.buf[f->pipe_buffer.head%size];
       f->pipe_buffer.head++;
       res++;
     }
     
     wakeup(&f->pipe_buffer.write_fd);
     release(&f->pipe_buffer.lock);
     return res;
   }    
}



int filewrite(int fd, char *buf, int bytes_written) { 
  struct proc* p =myproc();
  if(p->pftable[fd]==NULL)
    return -1;
  
   struct file_info* f=p->pftable[fd];
   
   if(!f->isPipe){  
     if(f->iptr==NULL)return -1;
     if(f->access_permission==O_RDONLY)return -1;
     return concurrent_writei(f->iptr, buf, f->offset, bytes_written);
   }else{
     // If passed in fd is not smae as pipe_write fd
     // if(p->pftable[fd]->pipe_buffer.write_fd!=fd){
     // return 0;
     //}
     //Pipe write
     acquire(&f->pipe_buffer.lock);
     int res = 0;
     int size = sizeof(f->pipe_buffer.buf);
     for(int i =0;i<bytes_written;i++){
       while((f->pipe_buffer.tail-f->pipe_buffer.head)==size){
	 wakeup(&f->pipe_buffer.read_fd);
	 sleep(&f->pipe_buffer.write_fd, &f->pipe_buffer.lock);
       } 
       if(f->ref==1){
	 release(&f->pipe_buffer.lock);
	 return -1;
       }
       f->pipe_buffer.buf[f->pipe_buffer.tail%size] = buf[i];
       f->pipe_buffer.tail++;
          res++;
     }
     
     wakeup(&f->pipe_buffer.read_fd);
     release(&f->pipe_buffer.lock);
     return res;
   }
}

int filedup(int fd) {
  struct proc* p = myproc();
  if(p->pftable[fd]==NULL)
    return -1;
  
  struct file_info f=*(p->pftable[fd]);
  if(f.isPipe == false && f.iptr==NULL)
    return -1;
  
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
  return 0;
}
