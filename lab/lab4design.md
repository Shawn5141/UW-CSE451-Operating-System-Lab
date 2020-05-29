# Lab 4 Design Doc: Filesystem

## Overview

### Part A 
#### Disk layout enable for write: 
- change the disk layout to enable files to be extended. Change open in `file.c` and `sysfile.c` in order to enable write.
- Use `bread`, `bwrite`, and `brelse`. Reference `readi` for formatting

#### Append to a file: 
- extend the size of a file. This is done by writing beyond the current end of the file. Need to allocate additional space for the file and then fill that space with the data being written.

#### Create files
- support the creation of files from the root directory. Create a new file when `O_CREATE` is passed to `open`

### Part B
#### synchronization support:
- ensure the writable file system that can supports multiprocessing and can handle concurrency

### Part C
#### Crash-safe file system
- journaling: each multi-block operation, write each block of the overall operation to a separate area of the disk (the log) before any of the changes are written to the main part of the disk (the inode table, the bitmap, etc.)

## In-depth Analysis and Implementation

### Disk File Write Enable  
- modify `kernel/file.c` to add cases for `O_WRONLY` AND `O_RDWR` (also for open in file.c)
- modify `writei` in `kernel/fs.c` so an inode can be used to write to disk
- use bread, bwrite, brelse to write to block (refer to readi)

### Disk File Append
- allocate additional space for the file. Use bitmap to find continuous blocks

### Disk File Create
- if file already exists, open files with given permissions. 
- update inode to represent the increase in file length
- update extent
- create if `O_CREATE` is passed to `open` system call
- if file dos not exist, create empty inode on disk, change root directory to add link to the new file, change bitmap on disk

### Synchronization Support
- use `locki(ip)` and `unlocki(ip)`
- always aquire the inode lock(`locki()`) before the block lock(acquired by calling `bread()`)
- remember not to acquire a `sleeplock` while holding a `spinlock`

### Crash-safe File System
- place log in the metadata area before the inodes (between inode and block cache layer) 
- use `bread` and `bwrite` interfaces
- implement helper fuctions `begin_tx()`and `commit_tx()` to update the log header on disk
- implement wrapper function `log_write` to write to the log region
- implement log_recover for replay log into disk

## Risk Analysis

## Unanswered Questions
- recover the transaction?
- Function and number of dirty_bits(B_DIRTY)
- How to ensure that Copy-On-Write traps are allowed in Kernel mode
## Time Estimation

- Disk Write Enable:               5 hr
- Disk Append:                     5 hr
- Disk Create:                     5 hr
- Synchronization support:         5 hr
- Crash-safe File System           15 hr
- Debug:                           10 hr
