#ifndef JEMALLOC_INTERNAL_DECLS_H
#define JEMALLOC_INTERNAL_DECLS_H

#include <math.h>

#  include <sys/param.h>
#  include <sys/mman.h>
#  if !defined(__pnacl__) && !defined(__native_client__)
#    include <sys/syscall.h>
#    if !defined(SYS_write) && defined(__NR_write)
#      define SYS_write __NR_write
#    endif
#    if defined(SYS_open) && defined(__aarch64__)
       /* Android headers may define SYS_open to __NR_open even though
        * __NR_open may not exist on AArch64 (superseded by __NR_openat). */
#      undef SYS_open
#    endif
#    include <sys/uio.h>
#  endif
#  include <pthread.h>
#  include <signal.h>
#  ifdef JEMALLOC_OS_UNFAIR_LOCK
#    include <os/lock.h>
#  endif
#  ifdef JEMALLOC_GLIBC_MALLOC_HOOK
#    include <sched.h>
#  endif
#  include <errno.h>
#  include <sys/time.h>
#  include <time.h>
#  ifdef JEMALLOC_HAVE_MACH_ABSOLUTE_TIME
#    include <mach/mach_time.h>
#  endif


#include <sys/types.h>

#include <limits.h>
#ifndef SIZE_T_MAX
#  define SIZE_T_MAX	SIZE_MAX
#endif
#ifndef SSIZE_MAX
#  define SSIZE_MAX	((ssize_t)(SIZE_T_MAX >> 1))
#endif
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#ifndef offsetof
#  define offsetof(type, member)	((size_t)&(((type *)NULL)->member))
#endif
#include <string.h>
#include <strings.h>
#include <ctype.h>
#  include <unistd.h>

#include <fcntl.h>

#endif /* JEMALLOC_INTERNAL_H */