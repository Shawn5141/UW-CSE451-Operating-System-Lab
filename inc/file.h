#pragma once

#include <extent.h>
#include <sleeplock.h>
#include <param.h>
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


struct pipe {
<<<<<<< HEAD
  struct spinlock lock;
=======
 struct spinlock lock; 
>>>>>>> 4713a3a673c2c8f07e71658e7ec3f483321eab84
 int read_fd;
 int write_fd;
 int head; 
 int tail;
 bool full;
 bool empty;
<<<<<<< HEAD
  bool middle;
=======
 bool middle;
>>>>>>> 4713a3a673c2c8f07e71658e7ec3f483321eab84
 char buf[2048];
}; 

//File info struct
struct file_info{
    int ref;//reference count
    struct inode* iptr;
    int offset;
    int prev_offset;
    int access_permission;// not sure whether it's a right type
    char* path;
    bool isPipe;
    struct pipe pipe_buffer;
};



//extern struct file_info ftable[NFILE];

//file.c
//int fileopen(char *path, int mode);

//int filedup(struct proc *p, struct file_info *f);

//int fileread(struct file_info *f, char *buf, int bytes_read);

//int filewrite(struct file_info *f, char *buf, int bytes_written);

//int fileclose(struct proc *p, struct file_info *f, int fd);

//int filestat(struct file_info *f, struct stat *fstat);
