#include <cdefs.h>
#include <fcntl.h>
#include <stat.h>
#include <user.h>

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

void concurrent_dup_test() {
  int i, pid, fd1, fd2, fd3, fd4;

  fd1 = open("concurrent_dup.txt", O_CREATE | O_RDWR);
  assert(fd1 == 3);

  for (i = 0; i < 50; i++) {
    pid = fork();
    if (pid == 0) {
      fd2 = dup(fd1);
      assert(fd2 == 4);
      fd3 = dup(fd1);
      assert(fd3 == 5);
      fd4 = dup(fd1);
      assert(fd4 == 6);

      sleep(50);

      assert(close(fd1) >= 0);
      assert(close(fd2) >= 0);
      assert(close(fd3) >= 0);
      assert(close(fd4) >= 0);
      exit();
    } else if (pid < 0) {
      error("fork() failed");
    }
  }

  for (i = 0; i < 50; i++) {
    wait();
  }

  // no more children
  if (wait() >= 0) {
    error("ghost child!");
  }

  assert(close(fd1) >= 0);
  printf(stdout, "concurrent dup test OK\n");
}

void concurrent_create_test() {
  int i, j, pid, fd, ret;
  char buf[5];

  fd = open("cc_create.txt", O_CREATE | O_WRONLY);
  assert(fd == 3);

  memset(buf, 0, 5);
  for (i = 0; i < 1000; i++) {
    assert(write(fd, buf, 5) == 5);
  }

  assert(close(fd) >= 0);

  for (i = 0; i < 50; i++) {
    pid = fork();
    if (pid == 0) {
      fd = open("cc_create.txt", O_CREATE | O_RDWR);
      assert(fd == 3);

      j = 100 * i;
      while (j > 0) {
        assert((ret = read(fd, buf, 5)) >= 0);
        j -= ret;
      }

      memset(buf, 65 + i, 5);
      for (j = 0; j < 20; j++) {
        assert(write(fd, buf, 5) == 5);
      }

      assert(close(fd) >= 0);
      exit();
    } else if (pid < 0) {
      error("fork() failed");
    }
  }

  for (i = 0; i < 50; i++) {
    wait();
  }

  // no more children
  if (wait() >= 0) {
    error("ghost child!");
  }

  fd = open("cc_create.txt", O_RDONLY);
  assert(fd == 3);

  for (i = 0; i < 50; i++) {
    for (j = 0; j < 100; j++) {
      while (read(fd, buf, 1) <= 0);
      assert(buf[0] == (i + 65));
    }
  }

  // EOF
  assert(read(fd, buf, 1) == 0);
  assert(close(fd) >= 0);

  printf(stdout, "cocurrent create test OK\n");
}

void concurrent_write_test() {
  int i, j, pid, fd;
  int map[50];
  char buf[5];

  fd = open("cc_write.txt", O_CREATE | O_WRONLY);
  assert(fd == 3);

  for (i = 0; i < 50; i++) {
    pid = fork();
    if (pid == 0) {
      for (j = 0; j < 5; j++) {
        buf[j] = i + 65;
      }

      for (j = 0; j < 20; j++) {
        assert(write(fd, buf, 5) == 5);
      }

      exit();
    } else if (pid < 0) {
      error("fork() failed");
    }
  }

  for (i = 0; i < 50; i++) {
    wait();
  }

  // no more children
  if (wait() >= 0) {
    error("ghost child!");
  }

  assert(close(fd) >= 0);
  fd = open("cc_write.txt", O_RDONLY);
  assert(fd == 3);

  for (i = 0; i < 50; i++) {
    map[i] = 0;
  }

  for (i = 0; i < 5000; i++) {
    while (read(fd, buf, 1) <= 0);
    map[buf[0] - 65]++;
  }

  // EOF
  assert(read(fd, buf, 1) == 0);
  assert(close(fd) >= 0);

  for (i = 0; i < 50; i++) {
    if (map[i] != 100)
      error("missing byte from child %d", i);
  }
  printf(stdout, "cocurrent write test OK\n");
}

void concurrent_read_test() {
  int i, j, pid, fd;
  uchar buf[5];

  fd = open("cc_write.txt", O_CREATE | O_WRONLY);
  assert(fd == 3);

  for (i = 0; i < 250; i++) {
    for (j = 0; j < 5; j++) {
      buf[j] = i;
    }
    for (j = 0; j < 4; j++) {
      assert(write(fd, buf, 5) == 5);
    }
  }

  assert(close(fd) >= 0);
  fd = open("cc_write.txt", O_RDONLY);
  assert(fd == 3);

  buf[0] = 0;
  for (i = 0; i < 50; i++) {
    pid = fork();
    if (pid == 0) {
      for (j = 0; j < 100; j++) {
        while (read(fd, buf + 1, 1) <= 0);
        if (buf[1] >= buf[0]) {
          buf[0] = buf[1];
        } else {
          error("read out of order: read %d before %d in child %d", buf[0], buf[1], i);
        }
      }
      exit();
    } else if (pid < 0) {
      error("fork() failed");
    }
  }

  for (i = 0; i < 50; i++) {
    wait();
  }

  // no more children
  if (wait() >= 0) {
    error("ghost child!");
  }

  assert(read(fd, buf, 1) == 0);
  assert(close(fd) >= 0);

  printf(stdout, "cocurrent read test OK\n");
}

int main(int argc, char *argv[]) {
  printf(stdout, "lab4test_b starting\n");

  concurrent_dup_test();
  concurrent_create_test();
  concurrent_write_test();
  concurrent_read_test();

  printf(stdout, "lab4test_b passed!\n");
  exit();
}

