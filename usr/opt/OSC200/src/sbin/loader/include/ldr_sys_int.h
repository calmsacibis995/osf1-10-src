/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: ldr_sys_int.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:16 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	ldr_sys_int.h : loader system interface functions
 *
 * OSF/1 Release 1.0
 */

/*
 *	NOTE: use of this include file requires the following others :
 *		"ldr_types.h"	- i.e. loader types
 */

#ifndef _H_LDR_SYS_INT
#define _H_LDR_SYS_INT

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/addrconf.h>

/* Flags for ldr_mmap */

#define LDR_PROT_NONE		PROT_NONE
#define	LDR_PROT_READ		PROT_READ
#define LDR_PROT_WRITE		PROT_WRITE
#define	LDR_PROT_EXEC		PROT_EXEC

/* flags contain sharing type, mapping type, and options */
/* sharing types: choose either SHARED or PRIVATE */

#define	LDR_MAP_SHARED		MAP_SHARED
#define	LDR_MAP_PRIVATE		MAP_PRIVATE

/* maping type; choose either FILE or ANON */

#define LDR_MAP_FILE		MAP_FILE
#define LDR_MAP_ANON		MAP_ANON
#define LDR_MAP_TYPE		MAP_TYPE


/* other flags */

#define LDR_MAP_FIXED       MAP_FIXED   /* map addr must be exactly as specified  */
#define LDR_MAP_VARIABLE    MAP_VARIABLE /* system can place new region */
#define LDR_MAP_HASSEMAPHORE	MAP_HASSEMAPHORE /* region may contain semaphores */
#define LDR_MAP_INHERIT     MAP_INHERIT	/* region is retained after exec */
#define LDR_MAP_UNALIGNED   MAP_UNALIGNED /* file offset may be non-page-aligned */


/* Flags for ldr_open */

#define	LDR_O_NONBLOCK		O_NONBLOCK
#define	LDR_O_APPEND		O_APPEND
#define	LDR_O_RDONLY		O_RDONLY
#define	LDR_O_WRONLY		O_WRONLY
#define	LDR_O_RDWR		O_RDWR
#define	LDR_O_CREAT		O_CREAT
#define	LDR_O_TRUNC		O_TRUNC
#define	LDR_O_EXCL		O_EXCL
#define	LDR_O_NOCTTY		O_NOCTTY

/* Flags for ldr_lseek */

#define	LDR_L_SET		0
#define	LDR_L_INCR		1
#define	LDR_L_XTND		2

/*
 *	File I/O routines
 */

extern ldr_file_t 
ldr_open __((const char *path, int flags));

extern int 
ldr_close __((ldr_file_t fhandle));

extern int 
ldr_read __((ldr_file_t fhandle, char *buf, unsigned nbytes));

extern int 
ldr_write __((ldr_file_t fhandle, char *buf, unsigned nbytes));

extern int 
ldr_stat __((const char *path, struct stat *buf));

extern int 
ldr_fstat __((ldr_file_t fhandle, struct stat *buf));

extern int 
ldr_lseek __((ldr_file_t fhandle, off_t offset, int whence));

/* Tell current file position */

#define ldr_ltell(fd)	(ldr_lseek((fd), (off_t)0, LDR_L_INCR))

extern int 
ldr_ftruncate __((ldr_file_t fhandle, off_t length));

extern int
ldr_unlink __((const char *path));

/* Grow the specified file, if required, to be at least the specified size.
 */

extern int
ldr_grow_file __((ldr_file_t fd, off_t new_size));

/*
 *	Mapping files and anonymous regions
 */

extern int 
ldr_mmap __((univ_t addr, size_t len, int prot, int flags,
	     ldr_file_t fhandle, off_t off, univ_t *mapped_addr));

extern int 
ldr_munmap __((univ_t addr, size_t len));

extern int 
ldr_msync __((univ_t addr, size_t len, int flags));

extern int
ldr_mprotect __((univ_t addr, size_t len, int prot));

extern int
ldr_mvalid __((univ_t addr, size_t len, int prot));


/* Duplicate the specified string into ldr_malloc'ed storage and return
 * the new storage.  Return NULL on error.
 */

extern char *
ldr_strdup __((const char *str));

/* Duplicate the specified string into ldr_heap_malloc'ed storage allocated
 * from the specified heap, and return the new storage.  Return NULL on error.
 */

extern char *
ldr_heap_strdup __((ldr_heap_t heap, const char *str));

/*
 * ldr_getpagesize() macro makes a getpagesize() system call.
 * Defined as a macro to isolate system call and make the code
 * extractable. Global variable ldr_vm_pagesize caches the VM
 * pagesize and is declared in utils/ldr_window.c .
 */

extern int ldr_vm_pagesize;	/* VM page size cache */

#define ldr_getpagesize() \
	(ldr_vm_pagesize != 0 ? ldr_vm_pagesize : \
	(ldr_vm_pagesize = getpagesize()))
	 

/* Get the address configuration record from the kernel.  Parse it by
 * filling in base addresses for regions defined as "adjacent" to other
 * regions (by chaining until you find a region with a specified address,
 * and using that region's address as the base).  Return a pointer to the
 * (static) address configuration record.  Returns LDR_SUCCESS on success
 * or negative error status on error.
 */

extern int
ldr_getaddressconf __((struct addressconf **conf));

/* Make a temporary file name, and create it.  Arguments are: pathname of
 * a file (or directory, must end in '/') in which to create the temp
 * file (may be NULL), and protection mode for new file.  Returns the
 * pathname of the temp file (in ldr_strdup'ed storage), and the open
 * file descriptor on the temp file.
 */

extern int
ldr_maketemp __((const char *loc, int mode, ldr_file_t *pfd, char **pfname));

/* Abort the process.  Don't bother with a core dump -- it won't
 * be useful anyway.
 */

extern void
ldr_abort __((void));

/* Simulate a breakpoint.  Used to return control to the debugger
 * after we complete loading a program, so it can debug it.
 */

extern void
ldr_bpt __((void));

#endif /* _H_LDR_SYS_INT */


