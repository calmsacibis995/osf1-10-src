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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/sysdep.h,v 1.1.2.2 92/12/11 08:36:06 devrcs Exp $ */
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* 	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND  MAY
 * 	BE USED OR COPIED ONLY IN ACCORDANCE WITH THE TERMS
 * 	OF SUCH LICENSE.
 *
 * 	COPYRIGHT    (c)    1988   BY   DIGITAL   EQUIPMENT
 * 	CORPORATION.  ALL RIGHTS RESERVED.
 *
 *
 * FACILITY:	EPIC equation editor
 *
 * ABSTRACT:	Sets operating-system-dependent compiliation parameters.
 *		Designed to be included FIRST in ALL *.c modules.
 *
 * NOTES:
 *  The following stuff is always dealt with here: 
	STATIC = symbol used to declare static function prototypes.
	WINID = X-Windows window ID.
	FULL_PROTO_OK = int f(int x); is OK.  Otherwise, you must use int f();.
	PROTO#(...) = function prototype macros.
	GLOBALVAR_, etc = global variable reference & declaration macros.
	CADDR_T = the datablock in an X-Windows callback.
	POINTER = generic pointer 
 *  The following stuff is conditionally dealt with here:
        SI_STR - header file for strcpy(), etc.
        SI_ALLOC - System independent memory mgt:  calloc(), etc.
	SI_IO - System independent I/O:  <io.h> or <unixio.h>
	SI_ATOI - System independent <stdlib.h>
	SI_TIME - System independent <time.h>	
	SI_MEM - System independent <stdlib.h>
	SI_PROC - System independent <process.h>:  getpid(), getenv()
	SI_FILE - System independent <file.h>
	SI_ERRNO - System independent <errno.h>
	SI_STAT - System independent <stat.h>
	SI_SORT - System independent <sort.h>.  For qsort,bsearch,etc.
 *
 * REVISION HISTORY:
 *	01-Nov-88 jnh:  added PTR2VOID and void handling.
 *	01-Nov-88 jnh:  prevent repeated inclusion of .h files (incl this one)
 *	31-Oct-88 jnh:  merged in all si_*.h files (repeated inclusion ignored).
 *	24-Oct-88 jnh:  copied from EPIC/Writer and added ULTRIX_CC.
 */

#ifndef _sysdep_
#define _sysdep_

/* the MAC's Lightspeed C compiler is the only one which does not allow 
    command-line symbols, so identify it by elimination. */
#ifndef VMS		/* CC on VMS on VAX */
#ifndef ULTRIX_VCC	/* VCC on ULTRIX on VAX */
#ifndef ULTRIX_PCC	/* CC on ULTRIX on VAX */
#ifndef EPIC_OS2	/* OS2/Presentation Manager on IBM PC-RT */
#ifndef MSDOS		/* MS/DOS on IBM PC */
#ifndef sparc
#define MAC_LSC		/* Lightspeed C on MacIntosh */
#endif
#endif
#endif
#endif
#endif
#endif


#define FILE_LEN 256	/* declare all filenames "char foofile[FILE_LEN]; */

#include <stdio.h>	/* the definition of "NULL" must be consistent */



#ifdef EPIC_OS2
#define POINTER void *
#include <os2def.h>
typedef HWND WINID;
#define STATIC static
#define FULL_PROTO_OK 0

#ifdef SI_ALLOC
#define	cfree(h)    free((char *)(h))
#endif

#ifdef SI_STR
#include <string.h>
#endif
#ifdef SI_IO
#include <io.h>
#endif

#ifdef SI_PROC
#include <stdlib.h>	/* getenv() */
#include <process.h>	/* getpid() */
#endif

#ifdef SI_ATOI
#ifndef _stdlib_
#define _stdlib_
#include <stdlib.h>
#endif
#endif

#ifdef SI_MEM
#ifndef _stdlib_
#define _stdlib_
#include <stdlib.h>
#endif
#endif

#ifdef SI_FILE
#include <fcntl.h>
#endif

#ifdef SI_ERRNO
#include <errno.h>
#endif

#ifdef SI_STAT
#include <types.h>
#include <stat.h>
typedef struct stat stat_t;
#endif

#ifdef SI_SORT
#include <search.h>
#endif

#endif

#ifdef VMS
#define POINTER void *
typedef unsigned long WINID;
#define STATIC static
#define FULL_PROTO_OK 1

#ifdef SI_ALLOC
void *VAXC$MALLOC_OPT(size_t size);
void *VAXC$CALLOC_OPT(size_t nmemb, size_t size);
int VAXC$FREE_OPT(void *ptr);
int VAXC$CFREE_OPT(void *ptr);
void *VAXC$REALLOC_OPT(void *ptr, size_t size);
#define malloc VAXC$MALLOC_OPT
#define calloc VAXC$CALLOC_OPT
#define free VAXC$FREE_OPT
#define cfree VAXC$CFREE_OPT
#define realloc VAXC$REALLOC_OPT
#endif

#ifdef SI_STR
#include <string.h>
#endif

#ifdef SI_IO
#include <unixio.h>
#endif

#ifdef SI_ATOI
#define _stdlib_
#include <stdlib.h>
#endif

#ifdef SI_TIME
#include <time.h> 
#endif            

#ifdef SI_MEM
#ifndef _stdlib_
#define _stdlib_
#include <stdlib.h>
#endif
#endif

#ifdef SI_PROC
#ifndef _stdlib_
#define _stdlib_
#include <stdlib.h> /* getpid() should be here... */
#endif
#endif

#ifdef SI_FILE
#include <file.h>
#endif

#ifdef SI_ERRNO
#include <errno.h>
#endif

#ifdef SI_STAT
#include <stat.h>
#endif

#ifdef SI_SORT
#ifndef _stdlib_
#define _stdlib_
#include <stdlib.h>
#endif
#endif

#endif

#ifdef ULTRIX_VCC
#define POINTER void *
typedef unsigned long WINID;
#define STATIC static
#define FULL_PROTO_OK 1

#ifdef SI_STR
#include <string.h>
#endif

#ifdef SI_TIME
#define _types_
#include <types.h>
#include <time.h> 
#endif            

#ifdef SI_FILE
#include <file.h>
#endif

#ifdef SI_ERRNO
#include <errno.h>
extern volatile int noshare errno;	/* UNIX style error code */
#endif

#ifdef SI_STAT
#include <stat.h>
#ifndef _types_
#define _types_
#include <types.h>
#endif
#endif

#endif

#ifdef ULTRIX_PCC
#define POINTER char *
typedef unsigned long WINID;
#define STATIC static
#define FULL_PROTO_OK 0

#ifdef SI_STR
#include <string.h>
#endif

#ifdef SI_TIME
#define _types_
#include <types.h>
#include <time.h> 
#endif            

#ifdef SI_FILE
#include <file.h>
#endif

#ifdef SI_ERRNO
#include <errno.h>
extern volatile int noshare errno;	/* UNIX style error code */
#endif

#ifdef SI_STAT
#include <stat.h>
#ifndef _types_
#define _types_
#include <types.h>
#endif
#endif

#endif

#ifdef MAC_LSC
#define POINTER void *
#define STATIC
#define CADDR_T
#define FULL_PROTO_OK 1

#ifdef SI_STR
#include <strings.h>
#endif

#ifdef SI_IO
#include <unix.h>
#endif

#ifdef SI_ATOI
#include <unix.h>
#endif

#ifdef SI_MEM
#include <storage.h>
#endif

#ifdef SI_PROC
#include <unix.h>
#endif

/* Careful:  unix.h includes io.h (and errno.h?), but
 * io.h (and errno.h?) doesn't check for double inclusion. */
#ifdef SI_FILE
#include <unix.h>
#endif

#ifdef SI_ERRNO
#include <stdio.h>
#include <errno.h>
#endif

#ifdef SI_STAT
extern int	stat(char *, struct stat *);
struct stat
    {
	long		st_size;
	unsigned short	st_mode;	/* file "mode" DIR or REG */
	unsigned int	st_ctime; 	/* file creation time */
    };
#define S_IFDIR	1
#define S_IFREG	2
typedef struct stat stat_t;
#endif

#ifdef SI_SORT
#include <unix.h>
#endif
#endif

#ifdef sparc
#define POINTER void *
typedef unsigned long WINID;
#define STATIC static
#define FULL_PROTO_OK 0

#ifdef SI_STR
#include <string.h>
#endif

#ifdef SI_TIME
#define _types_
#include <types.h>
#include <time.h> 
#endif            

#ifdef SI_FILE
#include <file.h>
#endif

#ifdef SI_ERRNO
#include <errno.h>
extern volatile int noshare errno;	/* UNIX style error code */
#endif

#ifdef SI_STAT
#include <stat.h>
#ifndef _types_
#define _types_
#include <types.h>
#endif
#endif

#if !defined(RAND_MAX)
#define RAND_MAX 0x7fff
#endif

#endif

#ifndef FULL_PROTO_OK
#define FULL_PROTO_OK 0
#endif

#if FULL_PROTO_OK
/* PROTO1(char *my_malloc,int nb) ==> char *my_malloc (int nb) */
#define PROTO0(f) f ()
#define PROTO1(f,a1) f (a1)
#define PROTO2(f,a1,a2) f (a1, a2)
#define PROTO3(f,a1,a2,a3) f (a1, a2, a3)
#define PROTO4(f,a1,a2,a3,a4) f (a1, a2, a3, a4)
#define PROTO5(f,a1,a2,a3,a4,a5) f (a1, a2, a3, a4, a5)
#define PROTO6(f,a1,a2,a3,a4,a5,a6) f (a1, a2, a3, a4, a5, a6)
#define PROTO7(f,a1,a2,a3,a4,a5,a6,a7) f (a1, a2, a3, a4, a5, a6, a7)
#define PROTO8(f,a1,a2,a3,a4,a5,a6,a7,a8) f (a1, a2, a3, a4, a5, a6, a7, a8)
#define PROTO9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9)
#define PROTO10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
#define PROTO11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)
#define PROTO12(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
#define PROTO13(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)
#define PROTO14(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14)
#define PROTO15(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) \
	f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)
#else
/* PROTO1(char *my_malloc,int nb) ==> char *my_malloc () */
#define PROTO0(f) f ()
#define PROTO1(f,a1) f ()
#define PROTO2(f,a1,a2) f ()
#define PROTO3(f,a1,a2,a3) f ()
#define PROTO4(f,a1,a2,a3,a4) f ()
#define PROTO5(f,a1,a2,a3,a4,a5) f ()
#define PROTO6(f,a1,a2,a3,a4,a5,a6) f ()
#define PROTO7(f,a1,a2,a3,a4,a5,a6,a7) f ()
#define PROTO8(f,a1,a2,a3,a4,a5,a6,a7,a8) f ()
#define PROTO9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9) f ()
#define PROTO10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) f ()
#define PROTO11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) f ()
#define PROTO12(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) f ()
#define PROTO13(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) f ()
#define PROTO14(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) f ()
#define PROTO15(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) f ()
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#ifndef GLOBAL
#define GLOBALVAR_(type, name, initval)	    extern type name
#define GLOBALXVAR_(type, name)		    extern type name
#define GLOBALARRAY_(type, name, count)	    extern type name []
#define GLOBALXARRAY_(type, name)	    extern type name []
#else
#define GLOBALVAR_(type, name, initval)	    type name = initval
#define GLOBALXVAR_(type, name)		    extern type name
#define GLOBALARRAY_(type, name, count)	    type name [ count ]
#define GLOBALXARRAY_(type, name)	    extern type name []
#endif
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#endif
