
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
//static struct file_info ftable[NFILE];

int fileopen(struct proc *p, char *path,int mode){
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
  
  for(int i=0; i<NFILE; i++) {
    if(ftable.valid[i] == 0) {
      struct file_info newf;
      newf.ref = 1;
      //      newf.type = ON_DISK;
      newf.iptr = iptr;
      newf.access_permission = mode;
      newf.gfd = i;
      ftable.pftable[i] = newf;
      ftable.valid[i] = 1;


      //Add process to table
      for(int j=0; j<NOFILE; j++) {
	if(p->pftable[j] == NULL) {
	  p->pftable[j] = &(ftable.pftable[i]);
	  
	  return j;
	}
      }

      //upon failure -- try to add to process file table
      ftable.valid[i] = 0;
      return -1;
    }
  }

  /*

  //need to allocate emtpy stat
  struct stat *istat;  //TODO Not sure I can create local varible here like this or I need to allocate some memory
  memset(&istat,0,sizeof(istat));
  // This function is inspired by thread on Ed : https://us.edstem.org/courses/399/discussion/28068
  if(iptr == 0) // if file dne 
    return -1;
  concurrent_stati(iptr,istat);
  if(iptr->type==1){ //TODO need to double check whehter it will return -1 if inode is directory //number can refer to stat.h in inc
       unlocki(iptr);
       return -1;
  }
  //make sure it is a "file" and not just read-only
  if(iptr->type==2 && mode!=O_RDONLY)
      return -1;

// find open slot on process open file table pftable
 int pfd = 0;//process file descriptor index
 for(pfd=0;pfd<NOFILE;pfd++){
    if(myproc()->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
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
  //ftable[gfd].offset =0;//should it be zero?
  ftable[gfd].access_permission= mode;//TODO Not sure what value should be assign here
  //Assign pointer to pftable in slot pfd
  myproc()->pftable[pfd] = &gfd;
  //Will always open device
  return pfd;
  */
  return -1;
}

int filestat(struct file_info *f, struct stat *fstat) {

}

int fileclose(struct proc *p, struct file_info *f, int fd) {
  /*
   if(f[*p->pftable[fd]].ref==1){
       f[*p->pftable[fd]].iptr=0;
       //       f[*p->pftable[fd]].mode=NULL;
   }
   f[*p->pftable[fd]].ref-=1;
   p->pftable[fd]=0;    
  */

   //Decrease reference count of file by 1
   //If ref count is 1
   if(f->ref > 1) {
     ftable.pftable[f->gfd].ref--;
   } else {
     ftable.valid[f->gfd] = 0;

   }

   //remove file from current process's file table
   p->pftable[fd] = NULL;
   return 0;
 
}

int fileread(struct file_info *f, char *buf, int bytes_read) {
  //Changes the offset of file_info struct
   return concurrent_readi(f->iptr,buf,f->offset,bytes_read);  
}


int filewrite(struct file_info *f, char *buf, int bytes_written) {
  //Changes the offset of file_info struct
  return concurrent_writei(f->iptr, buf, f->offset, bytes_written);
}

int filedup(struct proc *p, struct file_info *f) {

}
