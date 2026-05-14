/***************************************************************************//**
* \file cy_syslib_newlib_systemcalls.c
* \version 1.0
*
*  Description:
*   System Call implementations (mostly stubs) for GCC 'newlib'
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <sys/stat.h>

extern int __heap_start; // from linker file

__attribute__((weak)) void *_sbrk(int incr) {
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&__heap_start;
  }
  prev_heap = heap;

  heap += incr;

  return prev_heap;
}

__attribute__((weak)) int _close(int file) {
  return -1;
}

__attribute__((weak)) int _fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;

  return 0;
}

__attribute__((weak)) int _isatty(int file) {
  return 1;
}

__attribute__((weak)) int _lseek(int file, int ptr, int dir) {
  return 0;
}

__attribute__((weak)) void _exit(int status) {
  __asm("BKPT #0");
}

__attribute__((weak)) void _kill(int pid, int sig) {
  return;
}

__attribute__((weak)) int _getpid(void) {
  return -1;
}

__attribute__((weak)) int _write (int file, char * ptr, int len) {
  return len;
}

__attribute__((weak)) int _read (int file, char * ptr, int len) {
  return 0;
}


/* [] END OF FILE */
