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
 *	@(#)$RCSfile: fcntl.h,v $ $Revision: 4.2.11.6 $ (DEC) $Date: 1993/08/02 20:42:07 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * ORIGINS: 27, 3, 26
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_FCNTL_H_
#define _SYS_FCNTL_H_


#include <standards.h>
#include <sys/types.h>

/*
 * POSIX requires that certain values be included in fcntl.h and that only
 * these values be present when _POSIX_SOURCE is defined.  This header
 * includes all the POSIX required entries.
 */

#ifdef _POSIX_SOURCE

struct  flock   {
        short   l_type;
        short   l_whence;
        off_t   l_start;
        off_t   l_len;          /* len = 0 means until end of file */
        pid_t   l_pid;
};
/* file segment locking set data type - information passed to system by user */

/* file segment locking types */
#define F_RDLCK 1       /* Read (shared) lock */
#define F_WRLCK 2       /* Write (exclusive) lock */
#define F_UNLCK 8       /* Remove lock(s) */

#ifdef _NO_PROTO
extern int open();
extern int creat();
extern int fcntl();

#else 		/* use POSIX required prototypes */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
#ifndef _KERNEL
extern int open(const char *, int , ...);
extern int creat(const char *, mode_t );
extern int fcntl(int , int ,...);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _KERNEL */
#endif /* _NO_PROTO */

/* File status flags accessible to open(2) and fcntl(2) */
#define O_NONBLOCK 00000004	/* non-blocking I/O, POSIX style */
#define	O_APPEND   00000010  	/* append (writes guaranteed at the end) */

/* Mask for use with file access modes */
#define O_ACCMODE	3		

/* Flag values accessible to open(2) */
#define	O_RDONLY 00000000
#define	O_WRONLY 00000001
#define	O_RDWR	 00000002
 
/* Flag values accessible only to open(2) */
#define	O_CREAT	 00001000    /*  open with file create (uses third open arg)*/
#define	O_TRUNC	 00002000    /* open with truncation		*/
#define	O_EXCL	 00004000    /* exclusive open			*/
#define O_NOCTTY 00010000    /* POSIX REQUIRED */
#ifdef	_KERNEL
#define	O_DOCLONE 00020000    /* make a cloned device */
#endif

/* File descriptor flags used for fcntl() */
/* POSIX REQUIRED */
#define FD_CLOEXEC      1	/* Close this file during exec */

/* fcntl() requests */
#define	 F_DUPFD	0	/* Duplicate fildes		*/
#define	 F_GETFD	1	/* Get fildes flags		*/
#define	 F_SETFD	2	/* Set fildes flags		*/
#define	 F_GETFL	3	/* Get file flags		*/
#define	 F_SETFL	4	/* Set file flags		*/
#define	 F_GETLK	7	/* Get file lock	POSIX REQUIRED	*/
#define	 F_SETLK	8	/* Set file lock	POSIX REQUIRED	*/
#define	 F_SETLKW	9	/* Set file lock and waitPOSIX REQUIRED	*/

#endif /* _POSIX_SOURCE */

/*
 * File locking flags.  First 8 bits for various commands.  Rest
 * are for options to those commands.  Also need to define
 * end of file for file locking.
 */
#ifdef _KERNEL
#define GETFLCK         0x1     /* Get the file lock */
#define SETFLCK         0x2     /* Set the file lock */
#define CLNFLCK         0x4     /* Clean file locks */
#define VNOFLCK         0x8     /* File is locked */

#define RGETFLCK        0x10    /* Remote Get of file lock */
#define RSETFLCK        0x20    /* Remote Set of file lock */

#define SLPFLCK         0x100   /* Wait if blocked */
#define VRDFLCK         0x200   /* Read lock use */
#define VMANFLCK        0x400   /* Mandatory locking */
#define ENFFLCK         0x800   /* Enforcement mode locks */

#if	defined(__alpha)
#define MAXEND          0x7fffffffffffffff
#else
#define MAXEND          0x7fffffff
#endif /* __alpha */
#endif /* _KERNEL */


#ifdef _XOPEN_SOURCE
#include <sys/seek.h>
#include <sys/mode.h>

/* XOPEN REQUIRED  */
#define O_SYNC		00040000	/* synchronous write option */

#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE

/* Additional fcntl requests */

#define	 F_GETOWN	5	/* get async I/O owner		*/
#define	 F_SETOWN	6	/* set async I/O owner		*/

/*
 * Flock call.
 */
#define LOCK_SH         F_RDLCK       /* shared lock */
#define LOCK_EX         F_WRLCK       /* exclusive lock */
#define LOCK_NB         4       	/* don't block when locking */
#define LOCK_UN         F_UNLCK       /* unlock */


/*
 * Flag values accessible to open(2) and fcntl(2)
 */

/* O_NDELAY was defined as O_NONBLOCK.  The two definitions have been
 * seperated but the code has been modified so both flags still exhibit
 * the same behavior.  They will exhibit seperate behaviors in a future
 * release
 *
 * For DEC OSF/1 v1.3 these two flags are treated as the same except
 * by STREAMS.  The file and VFS layers will keep track of them separately
 * but drivers not knowing the difference should test for either being
 * set.
 */
#define O_DEFER		00000040	/* defered update		*/
#define O_NDELAY	00100000	/* Non-blocking I/O		*/


#define FNDELAY		O_NDELAY
#define	FAPPEND		O_APPEND
#define FASYNC		00100
#define	FCREAT		O_CREAT
#define	FTRUNC		O_TRUNC
#define	FSYNC		O_SYNC
#define	FEXCL		O_EXCL

#define	FNONBLOCK 	O_NONBLOCK
#define FDEFER		O_DEFER

#ifdef _KERNEL
#define FKERNEL         00200000        /* internal kernel use */
#define FVTEXT          01000000
#endif

/*
 * These may be AIX only values and should possibly be removed
 */

/* Flag values accessible only to open(2) */
#define	FOPEN		(-1)
#define	FREAD		00001
#define	FWRITE		00002
/*
 * DEC/OSF advisory flags for VOP_FSYNC calls.
 * These flags are additive with FWRITE.
 * Local filesystem types need not necessarily recognize them.
 * These flags are treated as mutually exclusive, and apply
 * to VREG files only.
 */
#define FWRITE_DATA     00004   /* Data blocks only */
#define FWRITE_METADATA 00010   /* Meta-Data blocks only */

#define FMARK           00020           /* mark during gc() */
#define FSHLOCK         00200           /* shared lock present */
#define FEXLOCK         00400           /* exclusive lock present */

/* FFCNTL is all the bits that may be set via fcntl. */
#define	FFCNTL	(FNONBLOCK|FNDELAY|FAPPEND|FSYNC|FASYNC) 

/* bits to save after open */
#define FMASK           (FSYNC|FASYNC|FAPPEND|FNDELAY|FNONBLOCK|FWRITE|FREAD)
#define FCNTLCANT       (FREAD|FWRITE|FMARK|FDEFER|FSHLOCK|FEXLOCK)

/* fcntl reqs for the lock mngr */
#define F_RGETLK        10      /* Test a remote lock */
#define F_RSETLK        11      /* Set or unlock a remote lock */
#define F_CNVT          12      /* Convert a fhandle to an open fd */
#define F_RSETLKW       13      /* Set or Clear remote lock(Blocking) */
#define	F_PURGEFS	14	/* Purge locks on fs (for ASE product) */

/* additional flag for l_type field of flock structure */
#define F_UNLKSYS  4   /* remove remote locks for a given system */

/* extended file segment locking set data type */
struct eflock {
        short   l_type;         /* F_RDLCK, F_WRLCK, or F_UNLCK */
        short   l_whence;       /* flag to choose starting offset */
        off_t   l_start;        /* relative offset, in bytes */
        off_t   l_len;          /* length, in bytes; 0 means lock to EOF */
        pid_t	l_pid;          /* returned with F_GETLK */
        pid_t	l_rpid;         /* Remote process id wanting this lock */
        int     l_rsys;         /* Remote system id wanting this lock */
	short   l_xxx;		/* grant lock flag in nfssrc4.2 lockd */
        int     l_cb;           /* IP address to callback		*/
};


#endif	/* _OSF_SOURCE	*/
#endif	/* _SYS_FCNTL_H_ */
