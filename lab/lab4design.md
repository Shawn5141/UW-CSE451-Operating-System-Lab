# Lab 3 Design Doc: Address Space Management

## Overview

### Part A 
#### Disk layout enable for write: 
- change the disk layout to enable files to be extended. Change open in file.c in order to enable write.

#### Append to a file: 
- extend the size of a file. This is done by writing beyond the current end of the file. Need to allocate additional space for the file and then fill that space with the data being written.

#### Create files
- create a empty inode on disk, change the root directory to add a link to the new file

### Part B
#### synchronization support:
- use locki(ip) and unlocki(ip). always aquire the inode
lock(locki()) before the block lock(acquired by calling bread())

### Part C
#### Crash-safe file system
- journaling: each multi-block operation, write each block of the overall operation to a separate area of the disk (the log) before any of the changes are written to the main part of the disk (the inode table, the bitmap, etc.)

## In-depth Analysis and Implementation

### Disk File Write Enable  

### Disk File Append

### Disk File Create

### Synchronization Support

### Crash-safe File System
- `begin_tx()`
- `commit_tx()`
- `log_write`
- `bread`
- `bwrite`    

## Risk Analysis

## Unanswered Questions


## Time Estimation

- Disk Write Enable:               5 hr
- Disk Append:                     5 hr
- Disk Create:                     5 hr
- Synchronization support:         5  hr
- Crash-safe File System           15 hr
- Debug:                           10 hr
