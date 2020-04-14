//
// File descriptors
//

#include <cdefs.h>
#include <defs.h>
#include <file.h>
#include <fs.h>
#include <param.h>
#include <sleeplock.h>
#include <spinlock.h>

struct devsw devsw[NDEV];

int void fileopen(char *path){
  iptr = namei(path); // find the inode with the path - increments reference count
  if(iptr == 0)
    return -1;
  
   if(iptr->type == ??) {
         unlocki(iptr);
         return -1;
                      }

   
   return  
}

