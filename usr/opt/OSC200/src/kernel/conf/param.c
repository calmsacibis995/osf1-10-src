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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: param.c,v $ $Revision: 4.3.21.10 $ (DEC) $Date: 1993/11/09 15:09:25 $";
#endif
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Modification History
 *
 * 27-Oct-91	Fred Canter
 *	Move rlimit structures here so data/stack size limits can
 *	be set in the kernel config file.
 *	Make System V IPC definitions configurable.
 *
 *  8-Oct-91	Phil Cameron
 *	Removed "ult_bin_isatty_fix" since it is no longer needed.
 *
 *  2-Jul-91	Jim McGinness
 *	Add "ult_bin_isatty_fix" as configureable parameter.  It is
 *	initialized here, used in bsd/tty.c, and can be altered
 *      as a boot parameter or through mipskopt().  It replaces the
 *	"ult_bin" parameter use in earlier base levels.
 *
 * 10-june-1991  Brian Harrigan
 *         Allow rt_preepmt_opt to be set/cleared at config time and
 *         configurable on kernels built from binaries
 * 6-June-1991   Brian Stevens
 *	  Allow maxuprc, bufcache, maxcallouts, and maxthreads (per task) to
 *	  be specified in the config file and to be configurable on kernels
 *	  built from binaries.
 *
 * 04-Jun-1991   Diane Lebel
 * 	Added max_nofile parameter for configurable number of
 *	per-process file descriptors.
 *
 * 15-May-1991   Paula Long
 *        Extended the size of the callout queue to take into account
 *        the fact that the sleep() and usleep() functions use the callout
 *        queue to put a process/thread to sleep.  Also Additional per-process
 *        timers were made available when RT_TIMER is defined, and that 
 */


/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <quota.h>
#include <confdep.h>

/*
 * NOTE: confdep.h MUST preceed param.h.
 *
 *	This allows overriding System V IPC default
 *	definitions from the kernel config file.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <ufs/quota.h>
#include <sys/file.h>
#include <sys/callout.h>
#include <sys/clist.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/utsname.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <rt_timer.h>
#include <rt_preempt.h>
#if RT_PREEMPT
#include <kern/lock.h>
#endif

#include <rt_sched_rq.h>
#if RT_SCHED_RQ
#include <sys/rtpriocntl.h>
#include <kern/sched.h>
#endif
#include <lat.h>
#include <pty.h>

/*
 * System parameter formulae.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 * Compiled with -DTIMEZONE=x -DDST=x -DMAXUSERS=xx
 */

/*
 * The CLOCKS_PER_SEC (Posix) constant is defined per machine in
 * <machine/machlimits.h>. But if it isn't default to 100.
 * But on alpha, "hz", "tick", "fixtick", and "tickadj" are initialized at
 * bootup in aloha_init.c.
 */

#ifdef __alpha
int hz;
int tick;
int fixtick;
int tickadj;
#else /* mips */

#if !defined(CLOCKS_PER_SEC)
#define CLOCKS_PER_SEC 100
#endif

int	hz = CLOCKS_PER_SEC;
int	tick = 1000000 / CLOCKS_PER_SEC;
                                        /* This is the amount of time which
					   must be added once every hz tick
					   (i.e. once per second!) */
int	fixtick = 1000000 - ((1000000/CLOCKS_PER_SEC) * CLOCKS_PER_SEC);
#if	CLOCKS_PER_SEC > 100
int	tickadj = 1;			/* can adjust CLOCKS_PER_SEC usecs per second */
#else
int	tickadj = 100 / CLOCKS_PER_SEC;		/* can adjust 100 usecs per second */
#endif
#endif /* __alpha */

int     maxdriftrecip = 1000;		/* This is a default which represents
					   a ridiculously poor clock which
					   drifts at a rate of 1 part in 1000,
					   most cpu clocks are good to between
					   1 part in 10000 (maxdriftrecip =
					   10000) and 2 parts in 100000
					   (maxdriftrecip = 50000). This value
					   should be established in some (clock)
					   hardware dependent module, e.g.
					   mc146818clock.c. for the PMAX */
struct	timezone tz = { TIMEZONE, DST };

struct utsname utsname = {
	"OSF1",
	"",
	"",
	"",
	MACHINE
};
		
#if !defined(MAXPROC)
#define NPROC (20 + 8 * MAXUSERS)
#else
#define NPROC MAXPROC
#endif
		
int	nproc = NPROC;

/* The following calculation was adopted from Ultrix */
#define NVNODE (NPROC + (2*MAXUSERS) + 128)

/* The following was the original calculation (from OSF) */
/*#define NVNODE (((((2*NPROC)+ 16 + MAXUSERS) + 32)*2) + (NMOUNT+((8*NMOUNT)*2)))*/

int	nvnode = NVNODE;
int	nfile = 16 * ((2*NPROC) + 16 + MAXUSERS) / 10 + 32;

/*
 * maxuprc is set from source by CHILD_MAX in sys/limits.h.  Allow it to be
 * overridden if maxuprc is set in the config file.  Future kernel code should
 * use the global int maxuprc, not MAXUPRC or CHILD_MAX.
 */
#if !defined(MAXUPRC)
int   maxuprc = CHILD_MAX;
#else
int   maxuprc = MAXUPRC;
#endif

#if !defined(AUTONICE)
int autonice = 0;
#else
int autonice = 1;
#endif

#if !defined(BUFCACHE)
int   bufcache = 3;
#else
int   bufcache = BUFCACHE;
#endif

#if	!defined(UBCMINPERCENT)
int	ubc_minpercent = 10;
#else
int	ubc_minpercent = UBCMINPERCENT;
#endif

#if	!defined(UBCMAXPERCENT)
int	ubc_maxpercent = 100;
#else
int	ubc_maxpercent = UBCMAXPERCENT;
#endif

#if	!defined(WRITEIO_KLUSTER)
vm_size_t vm_max_wrpgio_kluster = 32*1024;
#else
vm_size_t vm_max_wrpgio_kluster = WRITEIO_KLUSTER;
#endif	

#if	!defined(READIO_KLUSTER)
vm_size_t vm_max_rdpgio_kluster = 16*1024;
#else
vm_size_t vm_max_rdpgio_kluster = READIO_KLUSTER;
#endif

#if !defined(MAXTHREADS)
int   maxthreads = 256; /* max threads per task */
#else
int   maxthreads = MAXTHREADS; /* max threads per task */
#endif

#if !defined(THREADMAX)
#define	THREADMAX 8192
#endif

#if !defined(TASKMAX)
#define	TASKMAX 8192
#endif

#define	PORTMAX		((TASKMAX * 3 + THREADMAX)	/* kernel */ \
				+ (THREADMAX * 2)	/* user */   \
				+ 20000)		/* slop for objects */
					/* Number of ports, system-wide */

#define	SETMAX		(TASKMAX + THREADMAX + 200)
					/* Max number of port sets */

int	threadmax = THREADMAX;	/* max number of threads system-wide */
int	taskmax = TASKMAX;	/* max number of tasks system-wide */
int	port_hash_max_num = 50 * PORTMAX;
int	port_max_num = PORTMAX;
int	port_reserved_max_num = PORTMAX;
int	set_max_num = SETMAX;

int   maxusers = MAXUSERS;
int   sys_v_mode = SYS_V_MODE;

#if RT_PREEMPT
/* #if RT
 * RT_PREEMPT_OPT is set at config time so that preemption can be
 * turned off in a kernel which has RT_PREEMPT compiled on.
*/
#if !defined(RT_PREEMPT_OPT)
int   rt_preempt_opt= 0; /* preemption is OFF*/
#else
int   rt_preempt_opt = 1; /* Preemption is ON */
#endif

/*
 * Setting RT_PREEMPT_ENFORCE will crash the system if the system
 * returns to user context with a simple lock held.
 */
#if !defined(RT_PREEMPT_ENFORCE)
int   rt_preempt_enforce = 0; /* panic on bad slock_count in OFF */
#else
int   rt_preempt_enforce = 1; /* panic on bad slock_count in ON */
#endif
/*
 * Setting RT_LOCK_DEBUG turns on simple lock tracing.
 */
#if !defined(RT_LOCK_DEBUG)
int rt_check_locks = 0;
#else
int rt_check_locks = 1;
#endif
/*
 * SLOCKTRACE determines the size of the slock trace buffer at build time.
 * This needs to be configurable because sometimes a bigger buffer is
 * needed to capture a bad lock event during heavy lock activity.
 */
#if !defined(SLOCKTRACE)
#define SLOCKTRACE 32
#endif
int slocktrace = SLOCKTRACE;
struct rt_lock_debug rt_slock_trace[SLOCKTRACE];
#endif /* RT_PREEMPT */

/*
 * #if RT
 * The callout queue needs to be extended to take into account the
 * use of the callout queue for putting a thread to sleep.  It must
 * also be extended when RT_TIMERS are included, since RT_TIMERS allow
 * TIMER_MAX timers per-process.
 */
#if !defined(MAXCALLOUTS)
#if RT_TIMER
int	ncallout = 16 + NPROC + (NPROC * TIMER_MAX) + THREADMAX;
#else
int	ncallout = 16 + NPROC + THREADMAX;
#endif                         
#else
int   ncallout = MAXCALLOUTS;
#endif

#if defined(NCLIST)
#define MAXCLISTS  NCLIST
#else
#if LAT || NPTY
#define MAXCLISTS  60 + (12 * MAXUSERS)
#else
#define MAXCLISTS  120
#endif
#endif
int	nclist = MAXCLISTS;
int	create_fastlinks = 1;
int	select_max_elements = 1024 + NPROC * 4;
int	select_chunk_elements = 256;
int	path_num_max = 64;
int	ucred_max = 128;

/*
 *  Note: nmount_max is *not* a hard limit on the total number of mounts (nfs,
 *  	ufs, s5).  It is the number passed to zinit() when initializing the
 *  	'mount_zone' zone.  This zone is dynamically expandable.
 */
int	nmount_max = 2 * NMOUNT;

int	nmount = NMOUNT;		/* max number of ufs mounts */

int	nchsize = NVNODE * 11 / 10;
int	inohsz = 512;
int	spechsz = 64;
int	nchsz = 128;
int	bufhsz = 512;
int	numcpus = 1;
#ifdef	multimax
int	nmbclusters;
#else
#ifndef __alpha
int     nmbclusters = NMBCLUSTERS;
#endif /* ifdef __alpha */
#endif
#if	QUOTA
int	nquota = (MAXUSERS * 9) / 7 + 3;
int	ndquot = NVNODE + (MAXUSERS * NMOUNT) / 4;
#endif
int ufs_blkpref_lookbehind = 8;

/*
 * The default number of per-process file descriptors is configurable.
 * The getdtablesize(2) system call should be used to obtain the
 * current limit.  open_max_soft is the default per-process limit.
 * A process can increase its soft limit up to its hard limit using 
 * setrlimit(2).  The default hard and soft limits below must be 
 * at least 64, and less than or equal to OPEN_MAX_SYSTEM in param.h.
 */

int open_max_hard = OPEN_MAX_HARD;
int open_max_soft = OPEN_MAX_SOFT;

/*
 * These are initialized at bootstrap time
 * to values dependent on memory size
 */

long	nbuf;


/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted
 * (if they've been externed everywhere else; hah!).
 */
struct	proc *proc, *procNPROC;
struct	vnode *vnode, *vnodeNVNODE;
struct	file *file, *fileNFILE;
struct 	callout *callout;
struct	cblock *cfree = 0;
struct	cblock *cfreelist = 0;
int	cfreecount = 0;
struct	buf *buf;
char	*buffers;
struct	namecache *namecache;
#if	QUOTA
struct	quota *quota, *quotaNQUOTA;
struct	dquot *dquot, *dquotNDQUOT;
#endif

/*
 * Establish current and maximum values for task's
 * data and stack sizes. If maxdsiz is specified in
 * the config file, then the define comes from confdep.h,
 * otherwise the defaults in vmparam.h are used. Same
 * for maxssiz, dfldsiz, and dflssiz.
 */
#include <sys/vmparam.h>
#include <vm/vm_tune.h>

struct rlimit vm_initial_limit_stack = { DFLSSIZ, MAXSSIZ };
struct rlimit vm_initial_limit_data = { DFLDSIZ, MAXDSIZ };
struct rlimit vm_initial_limit_core = { RLIM_INFINITY, RLIM_INFINITY };
struct rlimit vm_initial_limit_rss = { DFLRSS, MAXRSS };
struct rlimit vm_initial_limit_vas = { MAXVAS, MAXVAS };

/*
 * System V IPC definitions. Default values come from param.h
 * or user can override the defaults in the kernel config file
 * (in which case values come from confdep.h).
 */

/* messages */
struct msqid_ds msgque[MSGMNI];         /* msg queue headers */
struct msginfo  msginfo = {             /* message parameters */
                           MSGMAX,
                           MSGMNB,
                           MSGMNI,
                           MSGTQL
};
/* Following will need work when security is implemented. */
#if     SEC_ARCH
/*
 * Allocate space for the message queue tag pools. On systems that
 * allocate the message queue structures dynamically, the tag pools
 * should also be dynamically allocated at the same time as the
 * queues.
 */
tag_t           msgtag[MSGMNI * SEC_TAG_COUNT];
#endif

/* semaphores */
struct semid_ds sema[SEMMNI];           /* semaphore data structures */
struct sem      sem[SEMMNS];            /* semaphores */
struct seminfo  seminfo = {             /* semaphore information structure */
                           SEMMNI,
                           SEMMSL,
                           SEMOPM,
                           SEMUME,
                           SEMVMX,
                           SEMAEM
};

union {
        u_short         semvals[SEMMSL];        /* set semaphore values */
        struct semid_ds ds;                     /* set permission values */
        struct sembuf   semops[SEMOPM];         /* operation holding area */
} semtmp;

/* Following will need work when security is implemented. */
#if     SEC_ARCH
/*
 * Allocate space for the semaphore tag pools.  On systems that allocate
 * the semaphore structures dynamically, the tag pools should also be
 * dynamically allocated at the same time as the semaphores.
 */
tag_t           semtag[SEMMNI * SEC_TAG_COUNT];
#endif

/* shared memory */
struct  shminfo shminfo = {     /* shared memory info structure */
                SHMMAX,
                SHMMIN,
                SHMMNI,
                SHMSEG
};

struct  shmid_internal  shmem[SHMMNI];  /* shared memory headers */

/* Following will need work when security is implemented. */
#if     SEC_ARCH
/*
 * Allocate space for the shared memory tag pools.  On systems that allocate
 * the shared memory structures dynamically, the tag pools should also be
 * dynamically allocated at the same time.
 */
tag_t           shmtag[SHMMNI * SEC_TAG_COUNT];
#endif

/* 
 * vm tune parameters 
 * If any of the vm_tune parameters are defined in
 * the config file, then the define comes from
 * confdep.h, otherwise the defaults in vm_tune.h
 * are used.
 */

struct vm_tune vm_tune = {
		COWFAULTS,		/* Copy point */
		MAPENTRIES,		/* Maximum map entries */
		MAXVAS,			/* Maximum VAS for user map */
		MAXWIRE,		/* Maximum wired memory */
		HEAPPERCENT,		/* Percent of memory for heap */
		ANONKLSHIFT,		/* 128K anon page shift */
		ANONKLPAGES,		/* Use system default anon_klpages */
		VPAGEMAX,		/* Maximum vpage for umap */
		SEGMENTATION,		/* Segmentation on or off */
		UBCPAGESTEAL,		/* Steal from vnode clean list */
		UBCDIRTYPERCENT,	/* Percent dirty UBC buffers */
		UBCSEQSTARTPERCENT,	/* Start when ubc is this percent */
		UBCSEQPERCENT,		/* Percent sequential allocates */
		CSUBMAPSIZE,		/* Size of kernel copy map */
		UBCBUFFERS,		/* Maximum UBC buffers */
		SYNCSWAPBUFFERS,	/* Maximum synchronous swap buffers */
		CLUSTERMAP,		/* ckluster dup map size*/
		CLUSTERSIZE,            /* max cluster bp size */
		ZONE_SIZE,              /* zone_map size */
		KENTRY_ZONE_SIZE,       /* kentry_map size */
		SYSWIREDPERCENT,	/* max pct of wired memory system-wide*/
		ASYNCSWAPBUFFERS,	/* Maximum asynchronous swap buffers */
		INSWAPPEDMIN		/* minimum inswapped ticks */
		
};

/*
 * This is the size in bytes of the kernel memory area that is available
 * to the SVR4 versions of kmem_alloc() and kmem_zalloc().  See ddi_init()
 * for details.
 */
int ddi_map_size = 0x10000;

#if RT_SCHED_RQ
/*
 * The quantum table, indexed by SVR4 RT priority.  Note that this table must
 * be editted when the number of RT priorities change because of changes in
 * SVR4_RT_MAX and/or SVR4_RT_MIN.  The priorities are given as MACH
 * priorities, and (SVR4_RT_MAX < SVR4_RT_MIN) and (SVR4_RT_MIN <=
 * BASEPRI_HIGHEST-1).
 */

#define SVR4_RT_MAX	(0)
#define SVR4_RT_MIN	(BASEPRI_HIGHEST-1)

int svr4_rt_max	= SVR4_RT_MAX;
int svr4_rt_min	= SVR4_RT_MIN;

struct rt_default_quantum rt_default_quantum[SVR4_RT_MIN - SVR4_RT_MAX + 1] =
{
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
  { 1, 0 },
};
#endif	/* RT_SCHED_RQ */
