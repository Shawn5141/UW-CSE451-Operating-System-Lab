#pragma once

#include <extent.h>
#include <sleeplock.h>

// in-memory copy of an inode
struct inode {
  uint dev;  // Device number
  uint inum; // Inode number
  int ref;   // Reference count
  int valid; // Flag for if node is valid
  struct sleeplock lock;

  short type; // copy of disk inode
  short devid;
  uint size;
  struct extent data;
};

// table mapping device ID (devid) to device functions
struct devsw {
  int (*read)(struct inode *, char *, int);
  int (*write)(struct inode *, char *, int);
};

extern struct devsw devsw[];

// Device ids
enum {
  CONSOLE = 1,
};
//File info struct
struct file_info{
    int ref;//reference count
    struct inode* iptr;
    int current_offset;
    int access_permission;// not sure whether it's a right type
};


int fileopen(char* path);

