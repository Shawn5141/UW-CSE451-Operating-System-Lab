#include <cdefs.h>
#include <fcntl.h>
#include <fs.h>
#include <memlayout.h>
#include <param.h>
#include <stat.h>
#include <syscall.h>
#include <trap.h>
#include <user.h>

char buf[8192];
int stdout = 1;
char* file_name = "newfile.txt";
int ROOT_DIR_START_SIZE = 400;
int DIRENT_SIZE = 16;
int INUM_START = 24;

#define error(msg, ...)                                                        \
  do {                                                                         \
    printf(stdout, "ERROR (line %d): ", __LINE__);                             \
    printf(stdout, msg, ##__VA_ARGS__);                                        \
    printf(stdout, "\n");                                                      \
    while (1) {                                                                \
    };                                                                         \
  } while (0)


// will return only if
// system is consistent. 
void check_system_consistent() {
  int fd;
  struct stat st;
  fd = open(file_name, O_RDONLY);
  if (fd < 0) {
    // File has not been created yet.
    // Ensure the root directory hasn't changed.
    fd = open(".", O_RDONLY);
    if (fstat(fd, &st) != 0) {
      error("New file does not exist: Cannot stat root directoy. Not consistent.");
    }
    if (st.size > ROOT_DIR_START_SIZE) {
      error("Root dir size grew, but data does not exist. Not consistent.");
    }
    if (st.size < ROOT_DIR_START_SIZE) {
      error("Root size shrunk, uh oh. Not consistent.");
    }
  } else {
    // File does exist. 
    if (fstat(fd, &st) != 0) {
      error("Cannot stat new file. Not consistent.");
    }
    // Ensure that the inode wasn't already allocated in the inodefile.
    // Note: This will error when the inodefile is updated, but the root inode wasn't.
    if (st.ino != INUM_START + 1) {
      error("New file inum is invalid, inum=%d, inum_expected=%d. Not consistent.", st.ino, INUM_START + 1);
    }
    // Ensure that the root directory grew by 1 dirent.
    fd = open(".", O_RDONLY);
    if (fstat(fd, &st) != 0) {
      error("Cannot stat root directoy. Not consistent.");
    }
    if (st.size != ROOT_DIR_START_SIZE + DIRENT_SIZE) {
      error("File exists but root directory size is incorrect, size=%d, size_expected=%d. Not consistent.", st.size, ROOT_DIR_START_SIZE + DIRENT_SIZE);
    }
  }
}

void check_before_file_creation() {
  int fd;
  fd = open(file_name, O_RDONLY);
  if (fd < 0) {
    printf(stdout, "File does not exist yet.\n");
    return;
  }
  printf(stdout, "File does exist before creation. Checking consistency.\n");
  // File has been created.
  check_system_consistent();
  printf(stdout, "File has been created and system is good!\n");
  printf(stdout, "lab4test_c passed!\n");
  exit();
}

void check_after_file_creation() {
  int fd;
  fd = open(file_name, O_RDONLY);
  if (fd < 0) {
    // File does not exist yet.
    error("File was not created. Not consistent.");
  }
  // File has been created.
  check_system_consistent();
  printf(stdout, "File has been created and system is good!\n");
}

// Tries to create a file.
// Will crash afters steps bwrite calls if (steps != 0).
void create_file(int steps) {
  int fd;
  if (steps)
    crashn(steps);
  fd = open(file_name, O_CREATE | O_RDWR);
  if (fd) {
    printf(1, "File created successfully\n");
  }
}

int main(int argc, char *argv[]) {
  int steps = 0;
  printf(stdout, "lab4test_c starting\n");
  if (argc > 1) {
    steps = atoi(argv[1]);
    printf(1, "crashing after %d bwrites\n", steps);
  }
  check_before_file_creation();
  create_file(steps);
  check_after_file_creation();
  printf(stdout, "lab4test_c passed!\n");
  exit();
}

