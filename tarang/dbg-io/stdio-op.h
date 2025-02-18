#ifndef _STDIO_OP_H_
#define _STDIO_OP_H_

#include  <errno.h>
#include  <stdio.h>
#include  <sys/stat.h>
#include  <sys/unistd.h>

void stdio_setup(void);
int _read(int file, char *data, int len);
int _write(int file, char *data, int len);
int _close(int file);
int _lseek(int file, int ptr, int dir);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _getpid_r(int file);
int _kill_r(int file);
caddr_t _sbrk(int incr);    /* Used by malloc to manage heap and stack collision */

/* following functions must be implemented by end application */
char stdio_get_char_bw(void);
void stdio_put_char_bw(char c);
#endif  /* _STDIO_OP_H_ */
