
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
 for(pfd=0;pfd<NOFILE;pfd++){
    if(myproc()->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
       break;
  }
 }
 int gfd =0;//global file descriptor index
  for(gfd=0;gfd<NFILE;gfd++){
    if(ftable[gfd].iptr==NULL){//TODO Not sure how to check is empty
       break;
    }
  }
  //Update ftable[gfd] file_info struct value 
  ftable[gfd].ref=0;
  ftable[gfd].iptr = iptr;
  ftable[gfd].current_offset =0;//should it be zero?
  ftable[gfd].access_permission= mode;//TODO Not sure what value should be assign here
  //Assign pointer to pftable in slot pfd
  myproc()->pftable[pfd] = &gfd;
  //Will always open device
  return pfd;

}

int filestat(struct file_info *f, struct stat *fstat) {

}

int fileclose(struct proc *p, struct file_info *f, int fd) {

}

int fileread(struct file_info *f, char *buf, int bytes_read) {

}

int filewrite(struct file_info *f, char *buf, int bytes_written) {

}

int filedup(struct proc *p, struct file_info *f) {

}
