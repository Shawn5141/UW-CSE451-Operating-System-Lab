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

#define error(msg, ...)                                                        \
  do {                                                                         \
    printf(stdout, "ERROR (line %d): ", __LINE__);                             \
    printf(stdout, msg, ##__VA_ARGS__);                                        \
    printf(stdout, "\n");                                                      \
    exit();                                                                    \
    while (1) {                                                                \
    };                                                                         \
  } while (0)

#define assert(a)                                                              \
  do {                                                                         \
    if (!(a)) {                                                                \
      printf(stdout, "Assertion failed (line %d): %s\n", __LINE__, #a);        \
      while (1)                                                                \
        ;                                                                      \
    }                                                                          \
  } while (0)

void overwrite(void) {
  int fd;

  printf(stdout, "overwrite...\n");
  strcpy(buf, "lab5 is the last 451 lab\n");
  fd = open("small.txt", O_RDWR);
  if (fd < 0) {
    error("Could not open small.txt with RW permissions");
  }
  // overwrite the original data.
  int n = write(fd, buf, strlen(buf) + 1);
  if (n != strlen(buf) + 1) {
    error("Did not write entire buffer. Wanted: %d, Wrote: %d\n", strlen(buf) + 1, n);
  }
  close(fd);

  fd = open("small.txt", O_RDONLY);
  read(fd, buf, 50);

  if (strcmp(buf, "lab5 is the last 451 lab\n") != 0)
    error("file content was not 'lab5 is the last 451 lab', was: '%s'", buf);

  close(fd);

  printf(stdout, "overwrite ok\n");
}

void append(void) {
  int fd;
  int old_size;

  printf(stdout, "append...  %d\n",strlen(buf));

  old_size = strlen(buf);
  
  fd = open("small.txt", O_RDWR);
  if (fd < 0) {
    error("Could not open small.txt with RW permissions");
  }

  // Advance the fd offset to 1 less than the size of file. 
  char garbo[old_size];
  int n = read(fd, &garbo, old_size - 1);
  if (n != old_size - 1) {
    error("read failed");
  }
  strcpy(buf, ", but this is just the beginning :(\n");
  // overwrite the last char, and append data.
  
  n = write(fd, buf, strlen(buf) + 1);
  if (n != strlen(buf) + 1) {
    error("Did not write entire buffer. Wanted: %d, Wrote: %d\n", strlen(buf) + 1, n);
  }
  printf(1,"wanted: %d,wrote %d ",strlen(buf)+1,n);
  close(fd);

  fd = open("small.txt", O_RDWR);
  read(fd, buf, 62);

  // If the content hasn't changed, then the dinode size is not being
  // updated for the file.
  if (strcmp(buf, "lab5 is the last 451 lab, but this is just the beginning :(\n") != 0)
    error("file content did not match expected, was: '%s'", buf);

  close(fd);

  printf(stdout, "append ok\n");
}


// Creates a new file.
// Writes 1 byte to it, reads 1 byte from it.
void filecreation(void) {
  int fd, n;
  printf(1, "filecreation...\n");

  if ((fd = open("create.txt", O_CREATE|O_RDWR)) < 0)
    error("create 'create.txt' failed");
    
  close(fd);

  // Reopen and write 1 byte.
  if ((fd = open("create.txt", O_RDWR)) < 0)
    error("open 'create.txt' after creation failed");
  memset(buf, 1, 1);
  n = write(fd, buf, 1);
  if (n != 1) {
    error("error writing to created file.\n");
  }
  close(fd);


  // Reopen and read 1 byte.
  if ((fd = open("create.txt", O_RDWR)) < 0)
    error("open 'create.txt' after creation failed");
  memset(buf, 0, 1);
  n = read(fd, buf, 1);
  if (n != 1) {
    error("error reading from created file.\n");
  }

  // Ensure read got the correct value.
  assert(buf[0] == 1);
  
  printf(1, "filecreation ok\n");

}


// Creates a file, writes and reads data.
// Data is written and read by 500 bytes to try
// and catch errors in writing. 
void onefile(void) {
  int fd, i, j;
  printf(1, "onefile...\n");

  if ((fd = open("onefile.txt", O_CREATE|O_RDWR)) < 0)
    error("create 'onefile.txt' failed");
    
  close(fd);
  if ((fd = open("onefile.txt", O_RDWR)) < 0)
    error("open 'onefile.txt' after creation failed");


  memset(buf, 0, sizeof(buf));
  for (i = 0; i < 10; i++) {
    memset(buf, i, 500);
    write(fd, buf, 500);
  }
  close(fd);
  if ((fd = open("onefile.txt", O_RDONLY)) < 0)
    error("couldn't reopen 'onefile.txt'");

  memset(buf, 0, sizeof(buf));
  for (i = 0; i < 10; i++) {
    if (read(fd, buf, 500) != 500)
      error("couldn't read the bytes for iteration %d", i);
    for (j = 0; j < 500; j++)
      assert(i == buf[j]);
  }

  printf(1, "onefile ok\n");
}

// four processes write different files at the same
// time, to test concurrent block allocation.
void fourfiles(void) {
  int fd, pid, i, j, n, total, pi;
  int num = 4;
  char *names[] = {"f0", "f1", "f2", "f3"};
  char *fname;

  printf(1, "fourfiles...\n");

  for (pi = 0; pi < num; pi++) {
    fname = names[pi];
    pid = fork();
    if (pid < 0) {
      error("fork failed.");
    }

    if (pid == 0) {
      fd = open(fname, O_CREATE | O_RDWR);
      if (fd < 0) {
        error("create failed\n");
      }

      memset(buf, '0' + pi, 512);
      for (i = 0; i < 12; i++) {
        if ((n = write(fd, buf, 500)) != 500) {
          error("write failed %d\n", n);
        }
      }
      exit();
    }
  }

  for (pi = 0; pi < num; pi++) {
    wait();
  }

  for (i = 0; i < num; i++) {
    fname = names[i];
    fd = open(fname, O_RDONLY);
    if (fd < 0) {
      error("file open failed for fname=%s\n", fname);
    }
    total = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
      for (j = 0; j < n; j++) {
        if (buf[j] != '0' + i) {
          error("wrong char, was %d should be %d\n", buf[j], '0' + i);
        }
      }
      total += n;
    }
    close(fd);
    if (total != 12 * 500) {
      error("wrong length i:%d, wanted:%d, %d\n", i, 12 * 500, total);
    }
  }

  printf(1, "fourfiles ok\n");
}

int main(int argc, char *argv[]) {
  printf(stdout, "lab4test_a starting\n");
  overwrite();
  append();
  filecreation();
  onefile();
  fourfiles();

  printf(stdout, "lab4test_a passed!\n");
  exit();
}

