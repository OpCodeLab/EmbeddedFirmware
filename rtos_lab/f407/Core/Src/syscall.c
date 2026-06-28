/*
 * syscall.c
 *
 *  Created on: 28 juin 2026
 *      Author: mhanzout
 */


/* syscalls.c — minimal newlib stubs for bare-metal */
#include <sys/stat.h>
#include <errno.h>
#include "main.h"
#ifdef MAIN6

int _close(int fd)           { (void)fd; errno = EBADF; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)          { (void)fd; return 1; }
int _lseek(int fd, int offset, int whence) { (void)fd;(void)offset;(void)whence; return 0; }
int _read(int fd, char *buf, int len) { (void)fd;(void)buf;(void)len; return 0; }


#endif
