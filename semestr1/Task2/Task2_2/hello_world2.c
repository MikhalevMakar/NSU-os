#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

enum {
    ERROR = -1,
    STDOUT = 1
};

static ssize_t my_write(const int _fd, void* _buf, const size_t _count) {
  register int    sys_write    asm("eax") = SYS_write;
  register int    fd           asm("edi") = _fd;
  register void*  buf          asm("esi") = _buf;
  register size_t count        asm("edx") = _count;

  asm("syscall");

  register int ret_value   asm("eax");

  if(ret_value < 0) {
      errno = -ret_value;
      return ERROR;
  }
  return ret_value;
}

int main(void) {
    ssize_t ret_value =  my_write(STDOUT, "hello world!\n", 14);
    return (ret_value == ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;
}
