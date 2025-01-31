/*!
 * @file    stdio-op.c
 * @brief   standard Input/Output operations. This file implements
 *          function bodies that will replace the exiting weak
 *          functions defined in stdio.h for usage of STDIO streams.
 */
#ifdef __GNUC__
#include "stdio-op.h"

#undef errno
extern int errno;
/*---------------------------------------------------------------------------*/
void
stdio_setup(void)
{
  /* Turn off buffers, so I/O occurs immediately */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}
/*---------------------------------------------------------------------------*/
int
_read(int file, char *data, int len)
{
  int bytes_read;
  if (file != STDIN_FILENO) {
    errno = EBADF;
    return -1;
  }
  for (bytes_read = 0; bytes_read < len; bytes_read++) {
    *data = stdio_get_char_bw();
    data++;
  }
  return bytes_read;
}
/*---------------------------------------------------------------------------*/
int
_write(int file, char *data, int len)
{
  int bytes_written;
  if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
    errno = EBADF;
    return -1;
  }
  for (bytes_written = 0; bytes_written < len; bytes_written++) {
    stdio_put_char_bw(*data);
    data++;
  }
  return bytes_written;
}
/*---------------------------------------------------------------------------*/
int
_close(int file)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
int
_lseek(int file, int ptr, int dir)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
int
_fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}
/*---------------------------------------------------------------------------*/
int
_isatty(int file)
{
  if ((file == STDOUT_FILENO) ||
      (file == STDIN_FILENO) ||
      (file == STDERR_FILENO)) {
      return 1;
  }
  errno = EBADF;
  return 0;
}
/*---------------------------------------------------------------------------*/
int 
_getpid_r(int file)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
int 
_kill_r(int file)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
#ifndef FREE_RTOS
register char *stack_ptr __asm("sp");
#endif  /* FREE_RTOS */

caddr_t __attribute__((weak))
_sbrk(int incr)
{
  extern char end __asm("end");
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == 0) {
    heap_end = &end;
  } 
  prev_heap_end = heap_end;

#ifdef FREE_RTOS
  char *min_stack_ptr;
  /* Use the NVIC offset register to locate the main stack pointer. */
  min_stack_ptr = (char*)(*(unsigned int *)*(unsigned int *)0xE000ED08);
  /* Locate the STACK bottom address */
  min_stack_ptr -= MAX_STACK_SIZE;
  if (heap_end + incr > min_stack_ptr) {
#else /* FREE_RTOS */
  if (heap_end + incr > stack_ptr) {
#endif  /* FREE_RTOS */
    extern void abort(void);
    _write(1, "_sbrk: Heap and stack collision\n", 32);
    abort();
    errno = ENOMEM;
    return (caddr_t) -1;
  }
  heap_end += incr;
  return (caddr_t) prev_heap_end;
}
/*---------------------------------------------------------------------------*/
char 
__attribute__((weak)) stdio_get_char_bw(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
void 
__attribute__((weak)) stdio_put_char_bw(char c)
{
  (void)c;
  return;
}
#endif  /* __GNUC__ */
