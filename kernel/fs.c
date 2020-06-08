// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xk/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include <cdefs.h>
#include <defs.h>
#include <file.h>
#include <fs.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>

#include <buf.h>

// there should be one superblock per disk device, but we run with
// only one device
struct superblock sb;

// Read the super block.
void readsb(int dev, struct superblock *sb) {
  struct buf *bp;

  bp = bread(dev, 1);
  memmove(sb, bp->data, sizeof(*sb));
  brelse(bp);
}

// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// range of blocks holding the file's content.
//
// The inodes themselves are contained in a file known as the
// inodefile. This allows the number of inodes to grow dynamically
// appending to the end of the inode file. The inodefile has an
// inum of 1 and starts at sb.startinode.
//
// The kernel keeps a cache of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The cached
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->flags.
//
// Since there is no writing to the file system there is no need
// for the callers to worry about coherence between the disk
// and the in memory copy, although that will become important
// if writing to the disk is introduced.
//
// Clients use iget() to populate an inode with valid information
// from the disk. idup() can be used to add an in memory reference
// to and inode. irelease() will decrement the in memory reference count
// and will free the inode if there are no more references to it,
// freeing up space in the cache for the inode to be used again.



struct {
  struct spinlock lock;
  struct inode inode[NINODE];
  struct inode inodefile;
} icache;

// Find the inode file on the disk and load it into memory
// should only be called once, but is idempotent.
static void init_inodefile(int dev) {
  struct buf *b;
  struct dinode di;

  b = bread(dev, sb.inodestart);
  memmove(&di, b->data, sizeof(struct dinode));

  icache.inodefile.inum = INODEFILEINO;
  icache.inodefile.dev = dev;
  icache.inodefile.type = di.type;
  icache.inodefile.valid = 1;
  icache.inodefile.ref = 1;

  icache.inodefile.devid = di.devid;
  icache.inodefile.size = di.size;
  icache.inodefile.data[0] = di.data[0];
  
  for(int i=1;i<EXTENT_N;i++){
    di.data[i].startblkno = 0;
    di.data[i].nblocks = 0;
    icache.inodefile.data[i] = di.data[i];
  }
  
  brelse(b);
}

void iinit(int dev) {
  int i;

  initlock(&icache.lock, "icache");
  for (i = 0; i < NINODE; i++) {
    initsleeplock(&icache.inode[i].lock, "inode");
  }
  initsleeplock(&icache.inodefile.lock, "inodefile");

  readsb(dev, &sb);
  cprintf("sb: size %d nblocks %d bmap start %d inodestart %d\n", sb.size,
          sb.nblocks, sb.bmapstart, sb.inodestart);

  init_inodefile(dev);
}


// Reads the dinode with the passed inum from the inode file.
// Threadsafe, will acquire sleeplock on inodefile inode if not held.
static void read_dinode(uint inum, struct dinode *dip) {
  int holding_inodefile_lock = holdingsleep(&icache.inodefile.lock);
  if (!holding_inodefile_lock)
    locki(&icache.inodefile);

  readi(&icache.inodefile, (char *)dip, INODEOFF(inum), sizeof(*dip));

  if (!holding_inodefile_lock)
    unlocki(&icache.inodefile);

}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not read
// the inode from from disk.
static struct inode *iget(uint dev, uint inum) {
  struct inode *ip, *empty;
  struct dinode dip;
  acquire(&icache.lock);

  // Is the inode already cached?
  empty = 0;
  for (ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++) {
    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
      ip->ref++;
      release(&icache.lock);
      return ip;
    }
    if (empty == 0 && ip->ref == 0) // Remember empty slot.
      empty = ip;
  }

  // Recycle an inode cache entry.
  if (empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->ref = 1;
  ip->valid = 0;
  ip->dev = dev;
  ip->inum = inum;

  release(&icache.lock);


  return ip;
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode *idup(struct inode *ip) {
  acquire(&icache.lock);
  ip->ref++;
  release(&icache.lock);
  return ip;
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode cache entry can
// be recycled.
void irelease(struct inode *ip) {
  acquire(&icache.lock);
  // inode has no other references release
  if (ip->ref == 1)
    ip->type = 0;
  ip->ref--;
  release(&icache.lock);
}

// Lock the given inode.
// Reads the inode from disk if necessary.
void locki(struct inode *ip) {
  struct dinode dip;

  if(ip == 0 || ip->ref < 1)
    panic("locki");

  acquiresleep(&ip->lock);

  if (ip->valid == 0) {

    if (ip != &icache.inodefile)
      locki(&icache.inodefile);
    read_dinode(ip->inum, &dip);
    if (ip != &icache.inodefile)
      unlocki(&icache.inodefile);

    ip->type = dip.type;
    ip->devid = dip.devid;

    ip->size = dip.size;
    for (int i = 0; i < EXTENT_N; i++) {
      ip->data[i] = dip.data[i];
      
    }


    ip->valid = 1;

    if (ip->type == 0)
      panic("iget: no type");
  }
}

// Unlock the given inode.
void unlocki(struct inode *ip) {
  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
    panic("unlocki");

  releasesleep(&ip->lock);
}

// threadsafe stati.
void concurrent_stati(struct inode *ip, struct stat *st) {
  locki(ip);
  stati(ip, st);
  unlocki(ip);
}

// Copy stat information from inode.
// Caller must hold ip->lock.
void stati(struct inode *ip, struct stat *st) {
  if (!holdingsleep(&ip->lock))
    panic("not holding lock");

  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->size = ip->size;
}

// threadsafe readi.
int concurrent_readi(struct inode *ip, char *dst, uint off, uint n) {
  int retval;

  locki(ip);
  retval = readi(ip, dst, off, n);
  unlocki(ip);

  return retval;
}
struct inode* concurrent_createi(char *path){
//1) Find an empty slot in the inodefile (it's an array of dinodes, remember) or append to the end 
 struct inode *inodefile = iget(ROOTDEV, INODEFILEINO);
 struct inode *ip;
  acquiresleep(&(inodefile->lock));
  ip = createi(inodefile,path);
  releasesleep(&(inodefile->lock));
  return ip;  
}

// Create inode in disk
struct inode* createi(struct inode* inodefile,char* path){
  
  cprintf("enter createi \n");
//2) Look through the bitmap to find a free extente 


//3) Use the found extents to construct a dinode and save this dinode to the place in the inodefile you found earlier 
 struct dinode din;
  din.type = T_FILE;
  din.devid = T_DEV;
  din.size = 0;
  for (int i = 0; i < EXTENT_N; i++) {
    din.data[i].startblkno =0;//getfreestartblkno(inodefile->dev);
    //cprintf("startblkno %d\n\n",din.data[i].startblkno);
    din.data[i].nblocks = 0;
  }
  cprintf("write to indoefile size %d\n");
 if (writei(inodefile, (char *) &din, inodefile->size, sizeof(struct dinode)) < 0)
    cprintf("failled to add dinode in sys_open\n");

  cprintf("after write to indoefile\n");
 //4) The inum is the index of the dinode within the inodefile 
  struct inode *rootino = iget(ROOTDEV, ROOTINO);
  struct dirent dir;
  cprintf("after iget \n");
  dir.inum = inodefile->size / sizeof(struct dinode);
  safestrcpy(dir.name, path, DIRSIZ);
  cprintf("dir num %d  path name %s inodefile->size %d sizeof dinode %d\n ",dir.inum,path,inodefile->size,sizeof(struct dinode));
 //5) Use that inum to add a dirent to the directory file
  
  cprintf("before write to rootfile\n");
   if (concurrent_writei(rootino, (char *) &dir, rootino->size, sizeof(struct dirent)) < 0)
    cprintf("failed to add dirent in sys_open\n");
  inodefile->size += sizeof(struct dinode);
  rootino->size += sizeof(struct dirent);
  update_dinode(inodefile); 
  update_dinode(rootino);
  
  icache.inodefile = *inodefile; 
  struct inode *ind = iget(ROOTDEV, dir.inum);
  return ind;

}

void update_dinode(struct inode* ip){
  struct inode *inodefile = iget(ROOTDEV, INODEFILEINO);
  struct dinode curr_dinode;
  read_dinode(ip->inum,&curr_dinode);
  if(ip->size!=curr_dinode.size){
     curr_dinode.size = ip->size;
     for(int i =0;i<EXTENT_N;i++){
        curr_dinode.data[i].startblkno = ip->data[i].startblkno;
        curr_dinode.data[i].nblocks = ip->data[i].nblocks;
     }
 if (writei(inodefile, (char *) &curr_dinode, ip->inum * sizeof(struct dinode),
           sizeof(struct dinode)) < 0)
    cprintf("failled to add dinode in sys_open\n");


  }
  


}

// Read data from inode.
// Returns number of bytes read.
// Caller must hold ip->lock.
int readi(struct inode *ip, char *dst, uint off, uint n) {
  uint tot, m;
  struct buf *bp;
  //cprintf("enter readi | off %d read_byte = %d inum %d \n",off,n,ip->inum);

  if (!holdingsleep(&ip->lock))
    panic("not holding lock");

  if (ip->type == T_DEV) {
    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].read)
      return -1;
    return devsw[ip->devid].read(ip, dst, n);
  }

  if (off > ip->size || off + n < off)
    return -1;
   //TODO need to double check
  //if (off + n > ip->size)
   // n = ip->size - off;
  
   struct extent *data = ip->data;
  uint size = 0;
  m = 0;

  // Move to the extent where offset is located.
     while (size + data->nblocks * BSIZE < off) {
          size += data->nblocks * BSIZE;
          data++;
       }
  // Offset for the specific extent.
     size = off - size;
  for (tot = 0; tot < n; tot += m, off += m, dst += m,size += m) {
    if (size < data->nblocks * BSIZE) {
    bp = bread(ip->dev,data->startblkno + size / BSIZE);
    m = min(n - tot, BSIZE - size % BSIZE);
    memmove(dst, bp->data + size % BSIZE, m);
    brelse(bp);
  }else{
      data++;
      m = 0;
      size = off - size;
    }
  }
  return n;
}

// threadsafe writei.
int concurrent_writei(struct inode *ip, char *src, uint off, uint n) {
  int retval;

  locki(ip);
  retval = writei(ip, src, off, n);
  unlocki(ip);

  return retval;
}

// Write data to inode.
// Returns number of bytes written.
// Caller must hold ip->lock.
int writei(struct inode *ip, char *src, uint off, uint n) {
  if (!holdingsleep(&ip->lock))
      panic("not holding lock");

  if (ip->type == T_DEV) {
    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].write)
      return -1;
    return devsw[ip->devid].write(ip, src, n);
  }
  // read-only fs, writing to inode is an error
  

//TODO Need to check if we need this condition 


    uint append = 0; 
  uint capacity = getCapacity(ip);
  if (off + n < off) 
    return -1;
  if (off + n > capacity) {
    append = (off + n) - capacity;
    n = capacity - off;
  }




  int i=0;
  uint size = 0;
  struct extent* data = ip->data;
  while(size+data->nblocks* BSIZE<off){
    size+=data->nblocks*BSIZE;
    data++;
  }
    size=off-size; 
  struct buf* bp;
  uint tot,m;
  for (tot = 0; tot < n; tot += m, off += m, src += m,size+=m) {
    m =0;
   if (size < data->nblocks * BSIZE) {
      bp =bread(ip->dev,data->startblkno+off/BSIZE);
      m = min(n - tot, BSIZE - size % BSIZE);
      memmove(bp->data + size % BSIZE,src, m);
      bwrite(bp);
      brelse(bp); 
    }else{
      data++;
      m = 0;
      size = off - size;
    }
    }
    if (append > 0) {
        return n + appendi(ip, src, append);
  }

    return n;   
}

int appendi(struct inode *ip, char *src, uint append) {
  uint tot, m;
  struct buf *bp;

  struct extent *data = ip->data;
  uint size = 0;

  while(data->nblocks != 0) {
    data++;
  }

  for (tot = 0; tot < append; tot += m, src += m, size += m, data++) {
    m = 0;

    if (data->nblocks == 0) {
      data->startblkno =  getfreestartblkno(ip->dev);
      data->nblocks = 8;
    }

    if (size < data->nblocks * BSIZE) {
      bp = bread(ip->dev, data->startblkno + size / BSIZE);
      m = min(append - tot, BSIZE - size % BSIZE);
      memmove(bp->data + size % BSIZE, src, m);
      bwrite(bp);
      brelse(bp);
    } else {
      data++;
      m = 0;
      size = 0;
    }
  }

  //log_commit_tx();
  return append;
}



// Directories

int namecmp(const char *s, const char *t) { return strncmp(s, t, DIRSIZ); }

struct inode *rootlookup(char *name) {
  return dirlookup(namei("/"), name, 0);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
struct inode *dirlookup(struct inode *dp, char *name, uint *poff) {
  uint off, inum;
  struct dirent de;

  if (dp->type != T_DIR)
    panic("dirlookup not DIR");

  for (off = 0; off < dp->size; off += sizeof(de)) {
//    cprintf("dirlookup off %d\n ",off);
    if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if (de.inum == 0)
      continue;
    if (namecmp(name, de.name) == 0) {
      // entry matches path element
      if (poff)
        *poff = off;
      inum = de.inum;
      return iget(dp->dev, inum);
    }
  }

  return 0;
}

// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char *skipelem(char *path, char *name) {
  char *s;
  int len;

  while (*path == '/')
    path++;
  if (*path == 0)
    return 0;
  s = path;
  while (*path != '/' && *path != 0)
    path++;
  len = path - s;
  if (len >= DIRSIZ)
    memmove(name, s, DIRSIZ);
  else {
    memmove(name, s, len);
    name[len] = 0;
  }
  while (*path == '/')
    path++;
  return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode *namex(char *path, int nameiparent, char *name) {
  struct inode *ip, *next;
  if (*path == '/'){
    ip = iget(ROOTDEV, ROOTINO);
  }else
    ip = idup(namei("/"));

  while ((path = skipelem(path, name)) != 0) {
    locki(ip);
    if (ip->type != T_DIR) {
      unlocki(ip);
      goto notfound;
    }

    // Stop one level early.
    if (nameiparent && *path == '\0') {
      unlocki(ip);
      return ip;
    }

    if ((next = dirlookup(ip, name, 0)) == 0) {
      unlocki(ip);
      goto notfound;
    }

    unlocki(ip);
    irelease(ip);
    ip = next;
  }
  if (nameiparent)
    goto notfound;

  return ip;

notfound:
  irelease(ip);
  return 0;
}

struct inode *namei(char *path) {
  char name[DIRSIZ];
  cprintf("namei %s",path);
  return namex(path, 0, name);
}

struct inode *nameiparent(char *path, char *name) {
  return namex(path, 1, name);
}


uint getfreestartblkno(int dev){
 struct buf *bp;
  //go through bitmap
  for (uint i = sb.inodestart - 1; i >= sb.bmapstart; i--) {
     bp =bread(dev,i);
   for(uint j=0;j<BSIZE;j++){
     // if block is availble, change to 0xff marked as occupied 
     if(bp->data[j]==0x00){
       bp->data[j]=0xff;
       bwrite(bp);
       brelse(bp);
       //cprintf("bitmap free block number %d\n",j*8+i*BSIZE+sb.inodestart); 
       return (sb.nblocks + sb.inodestart) 
               - ((sb.inodestart - 1 - i) * BSIZE)
               - (j + 1) * 8;

      }
      
    }
   brelse(bp);
  }
   return 0;

}


uint getCapacity(struct inode *ip) {
  uint capacity = 0;
  for (int i = 0; i < EXTENT_N; i++) {
    capacity += ip->data[i].nblocks * BSIZE;
  }
  return capacity;
}





