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
 *	@(#)$RCSfile: types.h,v $ $Revision: 4.3.15.6 $ (DEC) $Date: 1993/07/22 20:05:32 $
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
 * types.h
 *
 * Modification History:
 *
 * 27-Mar-92    David Metsky
 *      Added ssize_t definition for ISO/IEC 9945 (POSIX 1990)
 * 3-Jun-91     lebel
 *	Added support for > 64 open files per process.
 *
 * 4-Apr-91     Paula Long
 *      Added P1003.4 required extensions.  
 *
 */

/*
 * COMPONENT_NAME: type declaration header file
 *
 * ORIGIN: IBM, ATT, BSD
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *      (#)types.h     7.1 (Berkeley) 6/4/86
 */

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_
#include <standards.h>

#if     !defined(__LANGUAGE_ASSEMBLY__) && !defined(LOCORE)

#ifdef   _ANSI_C_SOURCE
/*
 * ANSI C required typedefs
 */

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef signed long     ptrdiff_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T

#ifndef __WCHAR_T_LEN
#    define __WCHAR_T_LEN 4
#endif

#if __WCHAR_T_LEN == 1
    typedef unsigned char wchar_t;
#elif __WCHAR_T_LEN == 2
    typedef unsigned short wchar_t;
#else
    typedef unsigned int  wchar_t;
#endif /* __WCHAR_T_LEN == ?? */

#endif /* _WCHAR_T */

#ifndef _WCTYPE_T
#define _WCTYPE_T
typedef unsigned int wctype_t;
#endif

#ifndef _FPOS_T
#define _FPOS_T
typedef long            fpos_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef int            time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef int             clock_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#endif   /* _ANSI_C_SOURCE */


#ifdef   _POSIX_SOURCE

#ifndef _SSIZE_T
#define _SSIZE_T
typedef long                    ssize_t; /* Required for ISO/IEC 9945-1:1990 */
#endif

/*
 * shorthand type definitions for unsigned storage classes
 */
typedef	unsigned char	uchar_t;
typedef	unsigned short	ushort_t;
typedef	unsigned int	uint_t;
typedef unsigned long	ulong_t;
typedef	volatile unsigned char	vuchar_t;
typedef	volatile unsigned short	vushort_t;
typedef	volatile unsigned int	vuint_t;
typedef volatile unsigned long	vulong_t;

#ifdef _OSF_SOURCE

#if	defined(__vax__) || defined(__ns32000__) || defined(__i386__)
typedef	struct	{ int r[1]; } *physadr_t;
typedef	struct	label_t	{
	int	val[14];
} label_t;
#endif	/* defined(__vax__) || defined(__ns32000__) || defined(__i386__) */

#ifdef	__ibmrt__
typedef	struct	{ int r[1]; } *physadr_t;
typedef	struct	label_t	{
	int	val[16];
} label_t;
#endif	/* __ibmrt__ */

#if	defined(__mc68000__) || defined(__mc68000)
typedef struct  { short r[1]; } *physadr_t;
typedef struct  label_t {
        int     val[13];
} label_t;
#endif	/* __mc68000__ */

#ifdef __sparc__
typedef struct  { int r[1]; } *physadr_t;
typedef struct label_t {
        int     val[2];
} label_t;
#endif	/* __sparc__ */

#ifdef	__mips__
typedef	struct	{ int r[1]; } *physadr_t;
/*
 * WARNING:
 * this must match the definition of kernel jmpbuf's in machine/pcb.h
 */
typedef	struct	label_t	{
	long	val[12];
} label_t;
#endif	/* __mips__ */

#ifdef	__alpha

typedef	struct	{ long r[1]; } *physadr_t;
/*
 * WARNING:
 * this must match the definition of kernel jmpbuf's in machine/reg.h
 */
typedef	struct	label_t	{
	long	val[10];
} label_t;
#endif	/* __alpha */

#endif /* _OSF_SOURCE */

typedef int		level_t;
typedef	int		daddr_t;	/* disk address */
typedef	char *		caddr_t;	/* "core" (i.e. memory) address */
typedef long *		qaddr_t;        /* should be typedef quad * qaddr_t; */
typedef char *          addr_t;
typedef	uint_t		ino_t;		/* inode number (filesystem) */
typedef short		cnt_t;
typedef int		dev_t;		/* device number (major+minor) */
typedef	int		chan_t;		/* channel number (minor's minor) */

#ifdef  _KERNEL
typedef ulong_t  off_t;			/* file offset */
#else   /* _KERNEL */
typedef long    off_t;			/* file offset */
#endif  /* _KERNEL */

typedef unsigned long	rlim_t;		/* resource limit */
typedef	int		paddr_t;
typedef	ushort_t	nlink_t;

#ifndef _KEY_T
#define _KEY_T
typedef int    		key_t;		/* ipc key type */
#endif

#ifndef _MODE_T
#define _MODE_T
typedef	uint_t		mode_t;		/* file mode */
#endif

#ifndef _UID_T
#define _UID_T
typedef uint_t		uid_t;		/* user ID */
#endif

#ifndef _GID_T
#define _GID_T
typedef uint_t		gid_t;		/* group ID */
#endif

typedef	void *		mid_t;		/* module ID	*/

#ifndef _PID_T
#define _PID_T
typedef	int		pid_t;		/* process ID */
#endif

typedef char		slab_t[12];	/* security label */
/*
 * The following type is for various kinds of identifiers.  The
 * actual type must be the same for all since some system calls
 * (such as sigsend) take arguments that may be any of these
 * types.  The enumeration type idtype_t defined in sys/procset.h
 * is used to indicate what type of id is being specified.
 */

typedef pid_t		id_t;		/* A process id,	*/
					/* process group id,	*/
					/* session id, 		*/
					/* scheduling class id,	*/
					/* user id, or group id.*/

#ifdef _OSF_SOURCE
#define	P_MYID	(-1)	/* a usually illegal value for IDs, but specifying
			   whatever the value is for my process */
#endif 

typedef ulong_t		shmatt_t;	/* for shmid_ds.shm_nattach */
typedef ulong_t		msgqnum_t;	/* for msqid_ds.msg_qnum */
typedef ulong_t		msglen_t;	/* for msqid_ds.msg_qbytes */

#ifndef _WINT_T
#define _WINT_T
        typedef unsigned int wint_t;         /* Wide character */
#endif

/* typedef for signal mask */
#ifndef _SIGSET_T
#define _SIGSET_T
typedef unsigned long	sigset_t;
#endif
#ifndef _KERNEL
/*
 * name conflict.
 *              /kernel/kern typedefs timer_t to something else.
 *              The file isn't exported.
 */
typedef long            timer_t;        /* timer id for _POSIX_4SOURCE */
#endif /* !defined(_KERNEL) */
#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE

#ifndef NULL
#define	NULL	0L
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef void (*sig_t)();

/*
 * Types used by dev_t annotation macros (see below)
 */
typedef uint_t	major_t;      /* major device number   */
typedef uint_t	minor_t;      /* minor device number   */
typedef uint_t	devs_t;       /* device-specific info  */
typedef uint_t	unit_t;       /* unit number of device */


/*
 * Basic system types and major/minor device constructing/busting macros.
 */
#define major(x)	((major_t)  (((dev_t)(x)>>20)&07777))
#define minor(x)	((minor_t)  ((dev_t)(x)&03777777))
#define makedev(x,y)	((dev_t)    (((major_t)(x)<<20) | (minor_t)(y)))

/*
 * Disk/Tape (SCSI/CAM - DSA) specific dev_t annotations macros.
 */
#define MAKEMINOR(u,d)  ((minor_t)  (((unit_t)(u)<<6) |(devs_t)(d)))
#define GETUNIT(dev)	((unit_t)   (minor(dev)>>6)&037777)
#define GETDEVS(dev)	((devs_t)   (minor(dev))&077)
#define MAKECAMMINOR(u,d) ((minor_t) MAKEMINOR((((u&0770)<<5)|((u&07)<<4)),d))
#define GETCAMUNIT(x)   ((unit_t) (((GETUNIT(x))>>5)&0770)|((GETUNIT(x)>>4)&07))
#define GETCAMTARG(x)   ((unit_t) ((x >> 3)&07))

#include <mach/machine/vm_types.h>

/*
 * shorthand type definitions for unsigned storage classes
 */
typedef	uchar_t		uchar;
typedef	ushort_t	ushort;
typedef	uint_t		uint;
typedef ulong_t		ulong;

typedef	physadr_t	physadr;


/* typedefs for BSD unsigned things */
typedef	uchar_t		u_char;
typedef	ushort_t 	u_short;
typedef	uint_t		u_int;
typedef	ulong_t		u_long;
typedef	vuchar_t	vu_char;
typedef	vushort_t 	vu_short;
typedef	vuint_t		vu_int;
typedef	vulong_t	vu_long;

#ifdef  _KERNEL
typedef struct  _quad { u_int val[2]; } quad;
#else   /* _KERNEL */
typedef struct  _quad { int val[2]; } quad;
#endif  /* _KERNEL */

typedef	long	swblk_t;
typedef u_long	fixpt_t;

/* We don't really want to include all of limits.h to get the 
   real value of this, so we leave this ugly constant here */
#define	NBBY	8		/* number of bits in a byte */

/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here is equal
 * to OPEN_MAX_SYSTEM in param.h since that defines the absolute maximum 
 * number of file descriptors that a process can open.
 */
#define MAX_NOFILE      4096
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	MAX_NOFILE
#endif

/* How many things we'll allow select to use. 0 if unlimited */
#define MAXSELFD        MAX_NOFILE
typedef int	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))


#endif   /* _OSF_SOURCE */

#endif  /* __LANGUAGE_ASSEMBLY__ */


#endif /* _SYS_TYPES_H_ */
