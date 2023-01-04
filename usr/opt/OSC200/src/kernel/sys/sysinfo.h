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
 *	@(#)$RCSfile: sysinfo.h,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/07/19 18:34:59 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/* sysinfo.h	5.2 87/01/09 18:25:51 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_SYSINFO_H_
#define _SYS_SYSINFO_H_

/*  NOTE:  An assembly-language version of these structures exists
 *            in "sysinfo.m4".  BE SURE to update that file whenever
 *            <sys/sysinfo.h> is changed!!!
 */

#include <sys/types.h>

struct sysinfo {
#define	CPU_NTIMES	4 	/* number of cpu times */
	time_t	cpu[CPU_NTIMES];   /* this array is updated every clock tick,
			     and keys off the state of the current running
			     process */
#define	CPU_IDLE	0 /* slot incremented whenever the 'wait' process
			     is the current running process */
#define	CPU_USER	1 /* slot incremented whenever the current running
			     process is executing in user mode */
#define CPU_KERNEL	2 /* slot incremented whenever the current running 
			     process is executing in kernel mode */
#define	CPU_WAIT	3 /* slot is always zero. This slot used to be
			     incremented whenever the current running process
			     was waiting for a block i/o request to
			     complete. Currently, whenever a process becomes
			     blocked, it is put to sleep and a new process
			     is made the current running process (i.e. processes
			     no longer maintain control of the cpu when they 
			     become blocked). */
	time_t	wait[3];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
#define	sysfirst	bread	/* first sysinfo variable - used by sysrates()
			  	   sysfirst define should be maintained as 
				   first non-array sysinfo variable */
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	pswitch;
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;  /* field is incremented by one whenever a 'fork'
			     is done */
	long	sysexec;  /* field is incremented by one whenever a 'exec'
			     is done */
	long	runque;   /* every second the process table is scanned to
			     determine the number of processes that are
			     ready to run. If that count is > 0 the
			     current number of ready processes is added 
			     to 'runque' (i.e. 'runque' is a cummulative
			     total of ready processes at second intervals). */
	long	runocc;   /* whenever 'runque' is updated, 'runocc'
			     is incremented by 1 (can be used to compute
			     the simple average of ready processes). */
	long	swpque;   /* every second the process table is scanned to 
			     determine the number of processes that are
			     inactive because they are waiting to be paged
			     in. If that count is > 0 then the current number
			     of processes waiting to be paged in is added
			     to 'swpque' (i.e. 'swpque' is a cummulative
			     total of processes waiting to be swapped in
			     at second intervals). */
	long	swpocc;   /* whenever 'swpque' is updated, 'swpocc' is 
			     incremented by one (can be used to compute
			     the simple average of processes waiting to be 
			     paged in).*/
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
	long    ksched;   /* field is incremented by one whenever a kernel
                             process is created */
	long    koverf;   /* field is incremented by one whenever an  attempt
			     is made to create a kernel process and:
				- the user has forked to their maximum limit
					       - OR -
				- the configuration limit of processes has been
				  reached */
	long    kexit;    /* field is incremented by one immediately after the
			     kernel process becomes a zombie */
	long    rbread;         /** remote read requests       **/
	long    rcread;         /** reads from remote cache    **/
	long    rbwrt;          /** remote writes              **/
	long    rcwrt;          /** cached remote writes       **/
#define	syslast		rcwrt	/* last sysinfo variable - used by sysrates() 
				   syslast define should be maintained as 
				   last non-array sysinfo variable */
};

#ifdef KERNEL
extern struct sysinfo sysinfo;
extern struct sysinfo sysrate;
extern struct sysinfo syshist;
#endif /* KERNEL */

struct syswait {
	short	iowait;
	short	physio;
};

#ifdef KERNEL
extern struct syswait syswait;
#endif /* KERNEL */

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
	long	sbi[5];
#define	SBI_SILOC	0
#define	SBI_CRDRDS	1
#define	SBI_ALERT	2
#define	SBI_FAULT	3
#define	SBI_TIMEO	4
};

#ifdef KERNEL
extern struct syserr syserr;
#endif /* KERNEL */

/*
 * The following structure defines the dump header which identifies the
 * start of a dump on the dump device.  It provides savecore with virtual
 * addresses of certain data items in the dump.
 */
#define DUMPINFO_SIGNATURE "!DuMpInFo!"
#define DUMPINFO_MAX 16
#define X_DUMPDEV       0 /* obsolete */
#define X_DUMPLO        1 /* obsolete */
#define X_TIME          2
#define X_DUMPSIZE      3
#define X_VERSION       4
#define X_PANICSTR      5
#define X_DUMPMAG       6 /* obsolete */
#define X_MSGBUF        7
#define X_BLBUFPADR     8
#define X_BLBUF         9
#define X_PARTDUMPMAG   10 /* obsolete */
#define X_PARTDUMP      11 /* obsolete */
#define X_BOOTEDFILE    12

struct dumpinfo {
	long		partial_dump;
	char		signature[12];
	int		csum;
	void		*addr[DUMPINFO_MAX];
};

/*
 *	The following contains constants used with the getsysinfo() and
 *	setsysinfo() system calls.
 *
 *	Both of these calls are operation driven; particular
 *	flavors of operation may use arguments, identifiers, flags, etc.
 *	to define the actual result.
 *
 */

/***************************************************************************
 ***************************************************************************
 **  WARNING, HAZARD, WATCH OUT, NOTICE!!!!!!!!!!!!!!
 **
 ** Whenever adding a new GSI or SSI number look in both <hal/hal_sysinfo.h>
 ** and <sys/sysinfo.h> to make sure that the function numbers remain unique!
 ** The defines for GSI/SSI appear in these two header files and can not
 ** overlap, otherwise the hal variant would never be called!
 **
 ***************************************************************************
 ***************************************************************************/


/*
 *	getsysinfo() operation types
 */

#define GSI_PROG_ENV	1	/* Return process compatibility mode of */
                                /* the process as defined in <sys/exec.h> */

#define GSI_MAX_UPROCS	2	/* Return the maximum number of processes */
                                /* allowed per user id */

#define	GSI_TTYP	3	/* Return the device number of the */
                                /* controlling terminal */

#define GSI_BOOTDEV	5	/* Return the bootdev string */
				/* which is used for install */


/* 
 * ULTRIX binary compatibility - On ULTRIX these return value of the flag 
 * that turns on/off printing the fixed up unaligned access message 
 */
#define GSI_UACSYS      6       /* get system wide flag */

#define GSI_UACPARNT    7	/* get parents */

#define GSI_UACPROC     8	/* get current proc */

#if	defined(mips) || defined(__alpha)
/*
 * Returns network interface boot type
 */
#define	GSI_BOOTTYPE	13	/* Network Interface boot type */
#endif

#define	GSI_LMF		9	/* License management facility (LMF) */

#if ULT_BIN_COMPAT

#define GSI_MMAPALIGN   12      /* support for mmap device drivers */

#define GSI_VPTOTAL	14      /* number of vector processors in system */

#define GSI_SCS		15	/* Systems Communications Services */

#define	GSI_PHYSMEM	19	/* Amount of physical memory in KB */

#define GSI_DNAUID	20	/* DNA UID genterator (UUID) */

#endif /* ULT_BIN_COMPAT */


/* All values from 1 - 22 are reserved. This is for compatibility */
/* with ULTRIX							  */

#define GSI_ROOTDEV     31     /* root device */

#define GSI_DUMPDEV     32     /* dump device */

#define GSI_SWAPDEV     33     /* dump device */

#define GSI_COMPAT_MOD	35	/* return compat_mod struct	*/

#define	GSI_STATIC_DEF	41	/* return Assign_entry struct	*/

#define GSI_SIA_PROC_CRED_VAL	37	/* return SIA proc auth val */

#define	GSI_CLK_TCK	42	/* return system clock hz */

#define GSI_IPRSETUP	48	/* return IP routing status for iprsetup */

#define GSI_DUMPINFO	49	/* return dump info for savecore utility */

/*
 *	setsysinfo() operation types
 */

#define	SSI_NVPAIRS	1	/* Use a list of name value pairs to modify */
                                /* pre-defined system variables */

#define	SSI_ZERO_STRUCT	2	/* Zero a pre-defined system structure */

#define	SSI_SET_STRUCT	3	/* Set a pre-defined system structure to */
                                /* supplied values */

/*
 *	setsysinfo() SSI_NVPAIRS variable names
 */

#define	SSIN_NFSPORTMON 1	/* A boolean which determines whether */
                                /* incoming NFS traffic is originating */
                                /* at a privileged port or not */

#define	SSIN_NFSSETLOCK	2	/* A boolean which determines whether NFS */
                                /* (daemon) style file and record locking */
                                /* is enabled or not */

#define SSIN_PROG_ENV	3	/* set prog environment, BSD, SYSV, POSIX */

/* see GSI_UACxxx */
#define SSIN_UACSYS	4	/* set system printing on/off */
#define SSIN_UACPARNT	5	/* set parent proc on/off */
#define SSIN_UACPROC	6	/* set current proc on/off */

/* All values from 1 - 8 are reserved for compatibility with ULTRIX */
#define SSI_LMF         7       /* License managment faciility (LMF) */

#if	ULT_BIN_COMPAT
#define SSI_LOGIN	8	/* Identify caller as a login process */
				/* (Sets SLOGIN flag in proc struct) */
#endif /* ULT_BIN_COMPAT */

#define SSI_SLIMIT      9      /* BVT */
#define SSI_ULIMIT     10      /* BVT */
#define SSI_DUMPDEV    11      /* Set dump device */

#define SSI_SIA_PROC_CRED_VAL	12	/* set SIA proc auth val */

#define SSI_IPRSETUP	13	/* set IP routing status for iprsetup */

#endif /* _SYS_SYSINFO_H_ */
