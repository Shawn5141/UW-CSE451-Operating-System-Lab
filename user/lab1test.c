#include <cdefs.h>
#include <fcntl.h>
#include <param.h>
#include <stat.h>
#include <stdarg.h>
#include <user.h>

// Will print an error message and loop
#define error(msg, ...)                                                        \
  do {                                                                         \
    printf(stdout, "ERROR (line %d): ", __LINE__);                             \
    printf(stdout, msg, ##__VA_ARGS__);                                        \
    printf(stdout, "\n");                                                      \
    while (1)                                                                  \
      ;                                                                        \
  } while (0)

// After error functionality is tested, this is just a convenience
#define assert(a)                                                              \
  do {                                                                         \
    if (!(a)) {                                                                \
      printf(stdout, "Assertion failed (line %d): %s\n", __LINE__, #a);        \
      while (1)                                                                \
        ;                                                                      \
    }                                                                          \
  } while (0)

int stdout = 1;

void testopen(void);
void testinvalidargs(void);
void smallfilereadtest(void);
void duptest(void);
void nofilestest(void);

int main() {

   if(open("console", O_RDWR) < 0){
 
    return -1;
   }
  dup(0);     // stdout
  dup(0);     // stderr
 // sleep(100); 
 // printf(stdout, "hello world\n");
 // sleep(10);
  //testopen();
  testinvalidargs();
  smallfilereadtest();
  duptest();
  while (1);

  testopen();
  testinvalidargs();
  smallfilereadtest();
  duptest();
  nofilestest();

  printf(stdout, "lab1 tests passed!\n");

  exit();
  return 0;
}

void testopen(void) {
  int fd, fd2;
  printf(stdout,"\ntest open\n");
  // test invalid open arguments.
  if (open("/other.txt", O_CREATE) != -1)
    error("created file in read only file system");

  if (open("/small.txt", O_RDWR) != -1 || open("/small.txt", O_WRONLY) != -1)
    error("tried to open a file for writing in read only fs");

  if (open("/other.txt", O_RDONLY) != -1)
    error("opened a file that doesn't exist");

  fd = open("/small.txt", O_RDONLY);
  if (fd == -1)
    error("cannot open small.txt");

  printf(stdout, "passed argument checking for open\n");

  // ensure opening the same file results with a
  // different file descriptor.
  fd2 = open("/small.txt", O_RDONLY);
  if (fd2 == -1)
    error("cannot open small.txt on 2nd try");
  if (fd == fd2)
    error("opening same file results with same file descriptor");

  printf(stdout, "passed opening same file twice.\n");

}

void testinvalidargs(void) {
  int fd, i;
  char buf[11];
  struct stat st;


  // read
  printf(stdout,"\nStart testinvalidargs\n");
  if (read(15, buf, 11) != -1)
    error("read on a non existent file descriptor");

  //printf(stdout,"pass first test\n");
  fd = open("console", O_WRONLY);
  assert(fd != -1);

  printf(stdout,"pass second test\n");
  if (read(fd, buf, 10) != -1)
    // NOTE: If the test hangs here it's because you're trying to read from
    // stdin (hint: shouldn't happen).
    error("able to read from a write only console");
  assert(close(fd) == 0);

  //printf(stdout,"pass third test\n");
  fd = open("/small.txt", O_RDONLY);

  if ((i = read(fd, buf, -100)) != -1)
    error("negative n didn't return error, return value was '%d'", i);
  //printf(stdout,"pass fourth test\n");
  if (read(fd, (char *)0xffffff00, 10) != -1)
    error("able to read to a buffer not in my memory region");

  printf(stdout, "passed argument checking for read\n");

  // write
  if (write(15, buf, 11) != -1)
    error("write on a non existent file descriptor");

  if (write(fd, buf, 11) != -1)
    error("able to write to a file in read only fs");

  printf(stdout, "passed argument checking for write\n");

  // stat
  if (fstat(15, &st) != -1)
    error("tried to fstat on a non existent file descriptor");

  if (fstat(stdout, &st) != 0) error("couldn't fstat on stdout");

  assert(st.type == T_DEV);
  assert(st.size == 0);

  if (stat("/small.txt", &st) != 0) error("couldn't stat on '/small.txt'");

  assert(st.type == T_FILE);
  assert(st.size == 26);

  printf(stdout, "passed argument checking for fstat\n");

  // dup
  if (dup(15) != -1)
    error("able to duplicated a non open file");

  printf(stdout, "passed argument checking for dup\n");

  // close
  if (close(15) != -1)
    error("able to close non open file");

  if (close(fd) > 0 && close(fd) != -1)
    error("able to close same file twice");

  printf(stdout, "passed argument checking for close\n");

}

void smallfilereadtest(void) {
  int fd, i;
  char buf[11];

  printf(stdout, "\nsmall file test\n");
  // Test read only funcionality
  fd = open("/small.txt", O_RDONLY);
  if (fd < 0)
    error("unable to open small file");
  printf(stdout, "open small file succeeded; ok return with fd =%d\n",fd);

  if ((i = read(fd, buf, 10)) != 10)
    error("read of first 10 bytes unsucessful was %d bytes", i);

  buf[10] = 0;
  if (strcmp(buf, "aaaaaaaaaa") != 0)
    error("buf was not 10 a's, was: '%s'", buf);
  printf(stdout, "read of first 10 bytes sucessful\n");

  if ((i = read(fd, buf, 10)) != 10)
    error("read of second 10 bytes unsucessful was %d bytes", i);

  buf[10] = 0;
  if (strcmp(buf, "bbbbbbbbbb") != 0)
    error("buf was not 10 b's, was: '%s'", buf);
  printf(stdout, "read of second 10 bytes sucessful\n");

  // only 25 byte file
  if ((i = read(fd, buf, 10)) != 6)
    error("read of last 6 bytes unsucessful was %d bytes", i);

  buf[6] = 0;
  if (strcmp(buf, "ccccc\n") != 0)
    error("buf was not 5 c's (and a newline), was: '%s'", buf);

  printf(stdout, "read of last 5 bytes sucessful\n");

  if (read(fd, buf, 10) != 0)
    error("read more bytes than should be possible");

  strcpy(buf, "stdout\n");

  if ((i = write(stdout, buf, 7)) != 7)
    error("wasn't able to write to stdout, return value was '%d'", i);

  printf(stdout, "write to stdout directly ok\n");

  if (close(fd) != 0)
    error("error closing fd");

  printf(stdout, "small file test ok\n");
}

void duptest(void) {
  int fd1, fd2, stdout_cpy;
  char buf[100];

  printf(stdout, "\ndup test\n");
  // Test read only funcionality
  fd1 = open("/small.txt", O_RDONLY);
  if (fd1 < 0)
    error("unable to open small file");

  if ((fd2 = dup(fd1)) != fd1 + 1)
    error("returned fd from dup was not the smallest free fd, was '%d'", fd2);

  // test offsets are respected in dupped files

  assert(read(fd1, buf, 10) == 10);
  buf[10] = 0;

  if (strcmp(buf, "aaaaaaaaaa") != 0)
    error("couldn't read from original fd after dup");

  if (read(fd2, buf, 10) != 10)
    error("coudn't read from the dupped fd");
  buf[10] = 0;

  if (strcmp(buf, "aaaaaaaaaa") == 0)
    error("the duped fd didn't respect the read offset from the other file.");

  if (strcmp(buf, "bbbbbbbbbb") != 0)
    error(
        "the duped fd didn't read the correct 10 bytes at the 10 byte offset");

  if (close(fd1) != 0)
    error("closing the original file");

  if (read(fd2, buf, 5) != 5)
    error("wasn't able to read from the duped file after the original file was "
          "closed");

  buf[5] = 0;
  assert(strcmp(buf, "ccccc") == 0);

  printf(stdout, "dup read offsets ok\n");

  if (close(fd2) != 0)
    error("closing the duped file");

  // test duping of stdout
  // should be fd1, because that is the first file I opened (and closed) earlier
  if ((stdout_cpy = dup(stdout)) != fd1)
    error("returned fd from dup that was not the smallest free fd, was '%d'",
          stdout_cpy);

  char *consolestr = "print to console directly from write\n";
  strcpy(buf, consolestr);

  if (write(stdout_cpy, consolestr, strlen(consolestr)) != strlen(consolestr))
    error("couldn't write to console from duped fd");

  assert(close(stdout_cpy) == 0);

  printf(stdout, "dup test ok\n");
}

void nofilestest() {
  int fd, tmpfd;

  printf(stdout, "nofiles test\n");
  fd = open("/small.txt", O_RDONLY);
  assert(fd != -1);

  for (tmpfd = fd + 1; tmpfd < NOFILE; tmpfd++) {
    int newfd = open("/small.txt", O_RDONLY);
    if (newfd != tmpfd)
      error("returned fd from open was not the smallest free fd, was '%d'",
            newfd);
  }

  // Opening should fail as we have reached the NOFILE limit
  if (open("/small.txt", O_RDONLY) != -1)
    error("opened more files than allowed");

  assert(close(NOFILE - 1) == 0);

  // Opening should work once there is a fd available
  int fd2 = open("/small.txt", O_RDONLY);
  if (fd2 == -1) error("unable to open file after an fd is available");

  assert(fd2 == NOFILE - 1);

  for (tmpfd = fd; tmpfd < NOFILE; tmpfd++) {
    assert(close(tmpfd) == 0);
  }

  printf(stdout, "nofiles test ok\n");
}
