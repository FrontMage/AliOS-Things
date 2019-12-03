/* Copyright Greenwaves Technologies 2019
 * LICENCE: GPLv2 (see COPYING)
 *
 * Author: Antoine Faravelon (antoine.faravelon@greenwaves-technologies.com)
 */

#ifndef _MACHINE_SYSCALL_H
#define _MACHINE_SYSCALL_H

#include <stdlib.h>
#include <stdint.h>

enum semihosting_operation_numbers {
	/*
	 * ARM/openocd semihosting operations.
	 * extracted from openocd semihosting_commong.h file
	 */
	SEMIHOSTING_ENTER_SVC = 0x17,	/* DEPRECATED */

	SEMIHOSTING_SYS_CLOCK = 0x10,
	SEMIHOSTING_SYS_ELAPSED = 0x30,
	
	SEMIHOSTING_SYS_ERRNO = 0x13,
	
	SEMIHOSTING_SYS_EXIT = 0x18,
	SEMIHOSTING_SYS_EXIT_EXTENDED = 0x20,
	// stat
	SEMIHOSTING_SYS_FLEN = 0x0C,
	SEMIHOSTING_SYS_GET_CMDLINE = 0x15,
	SEMIHOSTING_SYS_HEAPINFO = 0x16,
	SEMIHOSTING_SYS_ISERROR = 0x08,
	SEMIHOSTING_SYS_ISTTY = 0x09,
	
	// File operations
	SEMIHOSTING_SYS_OPEN = 0x01,
	SEMIHOSTING_SYS_CLOSE = 0x02,
	SEMIHOSTING_SYS_READ = 0x06,
	SEMIHOSTING_SYS_READC = 0x07,
	SEMIHOSTING_SYS_REMOVE = 0x0E,
	SEMIHOSTING_SYS_RENAME = 0x0F,
	SEMIHOSTING_SYS_SEEK = 0x0A,
	SEMIHOSTING_SYS_WRITE = 0x05,
	SEMIHOSTING_SYS_WRITEC = 0x03,
	// roughly a printf (print a string terminated by '\0')
	SEMIHOSTING_SYS_WRITE0 = 0x04,
	
	SEMIHOSTING_SYS_SYSTEM = 0x12,
	SEMIHOSTING_SYS_TICKFREQ = 0x31,
	SEMIHOSTING_SYS_TIME = 0x11,
	SEMIHOSTING_SYS_TMPNAM = 0x0D,
};


#define SEMIHOST_EXIT_SUCCESS 0x20026
#define SEMIHOST_EXIT_ERROR   0x20023

/* riscv semihosting standard: 
 * IN: a0 holds syscall number
 * IN: a1 holds pointer to arg struct
 * OUT: a0 holds return value (if exists)
 */
static inline long
__internal_semihost(long n, long _a1)
{
  register long a0 asm("a0") = n;
  register long a1 asm("a1") = _a1;

  // riscv magic values for semihosting
  asm volatile (
          ".option norvc;\t\n"
		  "slli    zero,zero,0x1f\t\n"
		  "ebreak\t\n"
		  "srai    zero,zero,0x7\t\n"
          ".option rvc;\t\n"
		: "+r"(a0) 
		: "r"(a1)
		);

  if (a0 < 0)
    return a0;
  else
    return a0;
}

/**
 * Printf a '\0' terminated string on host computer
 * Works both with gap8 semihosting and gvsoc
 *
 * @param[in]   print_string    pointer to a '\0' terminated string
 *
 */
void gap8_semihost_write0(const char *print_string);


/**
 * open file with name "name" and mode "mode" and return unix file descriptor
 * @param[in]   name    pointer to name of the file to be opened
 * @param[in]   mode    open mode (unix style)
 * @return  fd : On success, EIO : If an error occurred with any step
 */
int gap8_semihost_open(const char *name, int mode);

/**
 * Close unix file descriptor fd
 * @param[in]   fd    Unix file descriptor
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int gap8_semihost_close(int fd);

/**
 * Read len bytes from file descriptor fd to buffer "buffer"
 * @param[in]   fd      Unix file descriptor
 * @param[in]   buffer  pointer to buffer to store the data
 * @param[in]   len     number of bytes to read
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int gap8_semihost_read(int fd, uint8_t *buffer, int len);

/**
 * Write len bytes from buffer "buffer" to file corresponding to 
 * unix file descriptor fd
 * @param[in]   fd      Unix file descriptor
 * @param[in]   buffer  pointer to buffer to read data from
 * @param[in]   len     number of bytes to read
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int gap8_semihost_write(int fd, uint8_t *buffer, int len);

/**
 * Seek position (byte) in file corresponding to file descriptor fd
 * @param[in]   fd      Unix file descriptor
 * @param[in]   mode    position (byte) in file
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int gap8_semihost_seek(int fd, uint32_t pos);

/**
 * Returns the file length corresponding to unix file descriptor fd
 * @param[in]   fd      Unix file descriptor
 * @return  file length
 */
int gap8_semihost_flen(int fd);

/**
 * Singal exit to opencod or gvsoc
 * @param[in]   code    Exit code (SEMIHOST_EXIT_SUCCESS/SEMIHOST_EXIT_ERROR)
 */
void gap8_semihost_exit(int code);

#endif
