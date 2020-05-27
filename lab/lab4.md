# Lab 4: Filesystem
**Design Doc Due: 05/29/20 at 11:59pm. \
Complete Lab Due: 06/05/20 at 11:59pm.**

## Introduction
A restriction on the file system behavior in previous labs is that
files cannot be created, written to, or extended.  In this lab, we are going to
add this functionality to the file system. This is non-trivial because these
operations (often) involve updating multiple disk blocks. But we don't want
to leave the file system in an inconsistent state if the kernel crashes mid-update,
so these operations need to be performed in a crash-safe manner.

Another restriction in the implementation is that there is only one file
directory (`root` or '/'), which is the directory holding all information of reachable
files in the file system. For simplicity, there are no sub-directories
(all files will be added to the root directory). We will keep that restriction
in lab 4.

## Caution!
When your code writes to the QEMU simulated disk, QEMU will store the contents
of the disk in a file on your host system. The next time you run QEMU (i.e.,
run `make qemu` without `make clean`), it will used the modified disk, rather
than the original disk. Sometimes this will be what you want -- e.g., to test
crash safety. At other times, e.g., if you have a bug that leaves the disk in
a corrupted state, you will want to regenerate the disk to ensure
you start with the disk in a known state. To regenerate the disk run
`make clean` to delete the disk and then run `make` to generate a new disk
image. You are not required to write a file system checker ([`fsck`](https://linux.die.net/man/8/fsck)),
but you may find it useful to add some code to the xk initialization that
checks and warns the user if the disk is original or modified.

## Part A
### Disk layout
In xk's baseline implementation, all files are laid out on disk sequentially.
Your first task is to change the disk layout to enable files to be extended
from their initial size.

Take a look at `mkfs.c`. `mkfs.c` is a utility tool to generate the content
on the hard disk. This lab requires you to understand how files are laid out on disk.
Currently, xk's hard drive has a disk layout as the following:

	+------------------+  <- number of blocks in the hard drive
	|                  |
	|      Unused      |
	|                  |
	+------------------+  <- block 2 + nbitmap + size of inodes + cumulative extents
	|                  |
	|                  |
	|      Extents     |
	|                  |
	|                  |
	+------------------+  <- block 2 + nbitmap + size of inodes
	|                  |
	|      Inodes      |
	|                  |
	+------------------+  <- block 2 + nbitmap
	|      Bitmap      |
	+------------------+  <- block 2
	|   Super Block    |
	+------------------+  <- block 1
	|    Boot Block    |
	+------------------+  <- block 0

The boot block is used by the bootloader; the superblock describes how the disk
is formatted. The bitmap is a bit array with 1/0 for whether a particular disk block is used/free.
The inode table (`icache.inodefile`) is a contiguous region of the disk to save metadata about
each file; each inode on disk is (for now) 64 bytes and describes where to find the
disk blocks for that file, along with some additional information about the
file (e.g., file permissions on systems that support file permissions). The
inode table is stored as a file, with its own inode; this allows us to extend
the number of files in the system by appending to this inode table file.

The zeroth (`INODEFILEINO`) inode's data is by default the metadata for the inode file.
The next inode (`ROOTINO`) is the root directory. The first block of the inode
file is stored in the super block. This allows `iinit` to bootstrap the file
system.

Each directory entry (`struct dirent`) holds the file name and inode number -- the index
into the inode table. Note that we have no sub-directories, so you won't have
to implement sub-directories. Also note however that you will have to update the root
directory when adding a new file.

The data for each file is stored in the extents region. On boot, each file is
contiguously allocated. The lab will ask you to add the ability to extend
files, requiring you to use the disk region beyond the end of the pre-allocated
extents.

You are free to choose your own on-disk format. We only require you to maintain
compatibility with the file system call interface; you can change the file
layout however you choose. (We recommend you try to keep the changes minimal)

In particular, you will likely want to add fields to the `struct inode`
definition, e.g., to support multiple extents. You can use the vacant part of
the `struct dinode` in `inc/fs.h`, which denotes the on-disk representation of
an inode, to include more extents (e.g, a region of contiguous data on
disk), or you can create a block whose content is a list of block numbers
(i.e., an indirect block). If you modify `struct dinode`, you need to make
sure its size is a power of 2. This is to keep individual inodes from spanning
multiple disk blocks.


You can take a look at the `readi` implementation in `kernel/fs.c` to see
an example of using the offset and about how to handle files whose length is
longer than a single block.

You will also need to change the implementation of the `open` system call. In the
previous labs, the only allowed flag to open regular files is `O_RDONLY`. Now
you will need to support `O_RDWR` for read/write access to file. You can look
at `user/lab4test_a.c` to see the expected use-cases.


#### Question #1
Explain how `dirlookup` works.

### Append to a file
Once you can write to a file, the next step is to be able to extend the size
of a file. This is done by writing beyond the current end of the file. You will
need to allocate additional space for the file (e.g., with extra block pointers
or an extra extent pointer) and then fill that space with the data being
written.

### Create files
In addition to writing and appending to files, you will also need to support
creation of files from the root directory. New files are created by passing
`O_CREATE` flag with the corresponding R/W permission flags
(`O_RDONLY`, `O_WRONLY` or `O_RDWR`) to the `open` system call. If the files
to be created already exist, open() should just simply open the files
with the given permission.

You need to create a empty inode on disk, change the root directory to add a
link to the new file, and (depending on your disk layout) change bitmap on
disk. The inode file length itself will change, so don't forget to update this
as well. One of the benefits of implementing appending to a file is that you
can repurpose that to append the `inodefile`.

*Note*: File deletion is not required.

### Exercise
Enable writes to file system, appending to the end of a file, and creation of
files. All tests in `lab4test_a.c` should pass when run from the shell.

Expected Output:
```
$ lab4test_a
lab4test_a starting
overwrite...
overwrite ok
append...
append ok
filecreation...
filecreation ok
onefile...
onefile ok
fourfiles...
fourfiles ok
lab4test_a passed!
```

## Part B
### Synchronization in inode layer
Now we have a writable file system! Since xk supports multiprocessing, our
writable file system should handle any of the concurrency issues as well. If you
designed and implemented your Part A with synchronization in mind, you could
just run the test and see if your code works. If not, revisit your Part A code
and add synchronization support to your Part A. Think about when you would have
concurrency issues. What happens when you try to write to the same inode file?
What about extending the same inode file? Or creating new files at the same time? ...

*Hint*: Use `locki(ip)` and `unlocki(ip)` when appropriate.

*Note*: You can't acquire a `sleeplock` when you are holding a `spinlock`(read the
code in `sleep()` in `proc.c` and think about why). However, you can acquire a
`sleeplock` or a `spinlock` when you are holding a `sleeplock`. Be extra
careful when you want to hold multiple locks. You might run into a deadlock.

*Hint*: If you must hold multiple locks(there's a high chance you will), order
your lock acquires. For example, you should always aquire the inode
lock(`locki()`) before the block lock(acquired by calling `bread()`) if you
need to hold those two locks at the same time. This way, you would never run
into a deadlock.

### Exercise
Add synchronization support to Part A. All tests in `lab4test_b.c` should pass
when run from the shell.

Expected Output:
```
$ lab4test_b
lab4test_b starting
concurrent dup test OK
cocurrent create test OK
cocurrent write test OK
cocurrent read test OK
lab4test_b passed!
```

## Part C
### Crash-safe file system

Now that xk can add and modify files, the file system is still vulnerable to
crashes. A main challenge in file system design is to make the system
crash-safe: the file system state is always consistent no matter what the state
of execution is when the machine crashes.

For example, suppose you use the system call `write` to append a block of data to
a file. Appending to a file (may) require changing the bitmap to allocate a new
data block, changing the inode to hold the new file length, and writing the
actual data. The underlying disk system, however, writes a single block
(actually, an individual sector!) at a time. Without crash-safety, the file
could end up with inconsistent data: e.g., the bitmap having allocated the
block but the file doesn't use it, or vice versa.  Or the file length changed,
but the data not written so that a read to the end of the file returns garbage.
A crash-safe filesystem ensures that the file system is either entirely the new
data and entirely old data.

There are several ways to ensure crash-safety, and we don't specify which one
you should choose. We will talk about several in lecture. The most common
technique is to add journaling. The main idea is that for each multi-block
operation (e.g. modifying a single block as well as the necessary blocks of file
system data structures such as inodes), write each block of the overall 
operation to a separate area of the disk (the log) before any of the changes are
written to the main part of the disk (the inode table, the bitmap, etc.).
Once all parts of the operation are in the log, you can safely write the changes
back to their actual locations in the file system. If a failure occurs, on
kernel startup, you read the log to see if there were any completed operations 
(meaning all blocks of the multi-block operation were written to the log before
the crash); if so copy those changes back to the disk before continuing.
In other words, the contents of the log are idempotent -- able to be applied 
multiple times without changing the outcome.

The log can be allocated at the beginning of the disk. The best place to add the log region
is between the super block and the bitmap. Thus, we need to be able to ensure that
each operation fits in the log.  Generally, this is done by ensuring that
file create and single block write/append are atomic and crash safe.
Multi-block write/append is implemented as a sequence of one block atomic
and crash safe write/appends. The data format in directories and inodes are designed
so that updates apply to a single block.

If you choose to implement journaling, you can put the logging layer between
inode layer and block cache layer. In this way, you can use `bread`, `bwrite`
interfaces. You need to implement two helper functions `begin_tx()` and
`commit_tx()` to package a transaction. You will need transactions to perform
multi-block operations such as an atomic single block write (since writing a 
single block also involves modifying blocks of file system structures).
xk supports two transactions: file write and file create.
Additionally, you need to implement a wrapper function `log_write`. The 
difference between `log_write` and `bwrite` is that `log_write` does not write
to the actual disk location. `log_write` will write to the log region instead.
`commit_tx()` will  update the log header on disk, then go through the 
log region and write all of the updated blocks from the log regoin to their 
intended location on disk.
If the machine crashes before log header is modified,
the system behaves as if the multi-block transaction has not happened. If the 
machine crashes after the log header is written, then after the machine reboots,
xk has full knowledge of what the multi-block transaction is in the log so that
xk can enforce the transaction to succeed. Previously when xk writes to a disk 
block `disk_addr`, it does something like the following:

```c
// disk operation 1
buf = bread(dev, disk_addr);
memmove(buf->data, P2V(ph_addr), BSIZE);
bwrite(buf);
brelse(buf);
// more disk operations
// ....
```
Now, to use such a logging layer, xk should do following:
```c
begin_tx()
// disk operation 1
buf = bread(dev, disk_addr);
memmove(buf->data, P2V(ph_addr), BSIZE);
log_write(buf);
brelse(buf);
// more disk operations
// ....
commit_tx()
```
There is one caveat: if the reference count of a block is 0 in the block cache
layer, it is going to be reclaimed by the bcache (see `kernel/bio.c`). In the
above example, you can think of it as we want variable `buf` not to be
relcaimed after `brelse` because `commit_tx()` needs the content of `buf`. The
only way to prevent a block cache from eviction is to set it to be a dirty
block. For example, if you want to prevent buffer `b` from eviction you can set
the flags to be dirty by `b->flags |= B_DIRTY`. Once this is done, it will
remain in the cache until the flags are updated to not be dirty (Like, say at
the end of `commit_tx()`)?
+Or extend the same inode file? Or try to create new files at the same time?... 
+Use `locki(ip)` and `unlocki(ip)` when appropriate.

Testing for crash-safety is a bit complex. In your file system, there is a test
file called `user/lab4test_c.c`. The test code calls a helper system call
`crashn` which causes the system to reboot the OS after `n` disk operations.
The test attempts to create a file for different values of `n`. 

#### Question #8
File delete is not a required feature in lab4. Describe how you can implement
it in a crash-safe manner even if the file spans multiple blocks.

### Exercise
Build a crash-safe file system. Run `python2 crash_safety_test.py`. It should
print out `file system is crash-safe`.

Expected output:
```
clean finished.
make finished.
Running lab4test_c 30 times. test output=output.txt
finished i=1
finished i=2
finished i=3
finished i=4
finished i=5
finished i=6
finished i=7
finished i=8
finished i=9
finished i=10
finished i=11
finished i=12
finished i=13
finished i=14
finished i=15
finished i=16
finished i=17
finished i=18
finished i=19
finished i=20
finished i=21
finished i=22
finished i=23
finished i=24
finished i=25
finished i=26
finished i=27
finished i=28
finished i=29
finished i=30
killing qemu
qemu-system-x86_64: terminating on signal 15 from pid XXXXX (make)
file system is crash-safe
```

#### Question #9
For each member of the project team, how many hours did you spend on this lab?

Create a file `lab4.txt` in the top-level xk directory with
your answers to the questions listed above.

When you're finished, create a `end_lab4` git tag as described above so we know
the point at which you submitted your code.

## Debugging Help

* You may need to increase the number of blocks allocated for the root directory inode and inodefile inode if you do not allocate blocks dynamically. This can be done in `mkfs.c`.
* Ensure that Copy-On-Write traps are allowed in Kernel mode. This will be necessary for these lab tests.
* Crash safety tests relies on output from `lab4test_c`, so be careful when changing this file.
* Use the `hexdump` command in linux to manually observe the `fs.img` after terminating QEMU. 
    * You can use the `blockno` times `BSIZE` to index into the hexdump output
(Be sure to convert to hex).
    * For `onefile.txt` you can ensure the inode was written by looking at block `inum * sizeof(struct dinode) + 27 [the inodeno, this may change based on the log section] * BSIZE`.
    * You can also ensure the `direntry` is written to the extent of the root directory inode by viewing `dir_inode->extent.blockno * BSIZE`.
    * xk uses little endian. Read the lines left to right, but read individual bytes right to left.

## Files to look at

Here are some files you need for this lab:
* `file.c`: your file layer. You probably need to change `fileopen()` to allow
creation, and `fileread()`, `filewrite()`, `filestat()` to ensure synchronization.
* `fs.c`: inode layer. You need to implement create file, write file, and append
to file here. For Part B, you need to implement journaling here.
* `bio.c`: buffered I/O layer. `bread()`, `bwrite()` and `brelse()` are implemented here.
* `file.h`: definition of inode struct.
* `fs.h`: definition of dinode struct. If you change dinode struct, make sure the change
is reflected on inode struct as well.
* `extent.h`: definition of extent struct.
* `mfks.c`: this is the program that sets up your disk image. If you make changes
to dinode struct, you need to make sure the use of dinode in this file is still correct.

