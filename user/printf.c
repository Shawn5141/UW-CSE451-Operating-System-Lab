#include <cdefs.h>
#include <stat.h>
#include <stdarg.h>
#include <user.h>

static void putc(int fd, char c) { write(fd, &c, 1); }

static void printint64(int fd, int xx, int base, int sgn) {
  static char digits[] = "0123456789abcdef";
  char buf[32];
  int i;
  uint64_t x;

  if (sgn && (sgn = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sgn)
    buf[i++] = '-';

  while (--i >= 0)
    putc(fd, buf[i]);
}

static void printint(int fd, int xx, int base, int sgn) {
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if (sgn && xx < 0) {
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);
  if (neg)
    buf[i++] = '-';

  while (--i >= 0)
    putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void printf(int fd, char *fmt, ...) {
  char *s;
  int c, i, state;
  int lflag;
  va_list valist;
  va_start(valist, fmt);

  state = 0;
  for (i = 0; fmt[i]; i++) {
    c = fmt[i] & 0xff;
    if (state == 0) {
      if (c == '%') {
        state = '%';
        lflag = 0;
      } else {
        putc(fd, c);
      }
    } else if (state == '%') {
      if (c == 'l') {
        lflag = 1;
        continue;
      } else if (c == 'd') {
        if (lflag == 1)
          printint64(fd, va_arg(valist, int64_t), 10, 1);
        else
          printint(fd, va_arg(valist, int), 10, 1);
      } else if (c == 'x' || c == 'p') {
        if (lflag == 1)
          printint64(fd, va_arg(valist, int64_t), 16, 0);
        else
          printint(fd, va_arg(valist, int), 16, 0);
      } else if (c == 's') {
        if ((s = (char *)va_arg(valist, char *)) == 0)
          s = "(null)";
        for (; *s; s++)
          putc(fd, *s);
      } else if (c == '%') {
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }

  va_end(valist);
}
