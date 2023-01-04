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
 *	@(#)$RCSfile: table.h,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/11/11 20:44:15 $
 */ 
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
 * Copyright (c) 1986 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	_SYS_TABLE_H_
#define _SYS_TABLE_H_
#ifndef   ASSEMBLER
#include <standards.h>
#include <sys/types.h>
#include <mach/vm_prot.h>
#endif

#define TBL_TTYLOC		0	/* index by device number */
#define TBL_U_TTYD		1	/* index by process ID */
#define TBL_UAREA		2	/* index by process ID */
#define TBL_LOADAVG		3	/* (no index) */
#define TBL_INCLUDE_VERSION	4	/* (no index) */
#define TBL_FSPARAM		5	/* index by device number */
#define TBL_ARGUMENTS		6	/* index by process ID */
#define TBL_MAXUPRC		7	/* index by process ID */
#define TBL_AID			8	/* index by process ID */
#define TBL_MODES		9	/* index by process ID */
#define TBL_PROCINFO		10	/* index by proc table slot */
#define TBL_ENVIRONMENT		11	/* index by process ID */
#define TBL_SYSINFO		12	/* (no index) */
#define TBL_DKINFO		13	/* index by disk */
#define TBL_TTYINFO		14	/* (no index) */
#define TBL_MSGDS		15	/* index by array index */
#define TBL_SEMDS		16	/* index by array index */
#define TBL_SHMDS		17	/* index by array index */
#define TBL_MSGINFO		18	/* index by structure element */
#define TBL_SEMINFO		19	/* index by structure element */
#define TBL_SHMINFO		20	/* index by structure element */
#define TBL_INTR		21	/* (no index) */
#define TBL_SWAPINFO		22	/* index by vm_swap element */
#define TBL_SCALLS      	23      /* system call info table (no index) */
#define TBL_FILEINFO    	24      /* file access status (no index) */
#define TBL_TBLSTATS    	25      /* system tables status (no index) */
#define TBL_RUNQ        	26      /* run queue status (no index) */
#define TBL_BUFFER      	27      /* buffer activity (no index) */
#define TBL_KMEM      		28      /* kernel memory activity (no index) */
#define TBL_PAGING      	29      /* paging activity (no index) */
#define TBL_MAPINFO		31	/* process address map */
#define TBL_MALLOCBUCKETS	32	/* get malloc allocator kmembuckets */
#define TBL_MALLOCTYPES		33	/* get malloc allocator kmemtypes */
#define TBL_MALLOCNAMES		34	/* get malloc memory type names */

#define MSGINFO_MAX		0	/* max message size */
#define MSGINFO_MNB		1	/* max # bytes on queue */
#define MSGINFO_MNI		2	/* # of message queue identifiers */
#define MSGINFO_TQL		3	/* # of system message headers */
#define MSGINFO_SAR     	4       /* # of send and receive messages */

#define	SEMINFO_MNI		0	/* # of semaphore identifiers */
#define	SEMINFO_MSL		1	/* max # of semaphores per id */
#define	SEMINFO_OPM		2	/* max # of operations per semop call */
#define	SEMINFO_UME		3	/* max # of undo entries per process */
#define	SEMINFO_VMX		4	/* semaphore maximum value */
#define	SEMINFO_AEM		5	/* adjust on exit max value */
#define SEMINFO_OPT     	6       /* # of semaphore operations */

#define SHMINFO_MAX		0	/* max shared memory segment size */
#define SHMINFO_MIN		1	/* min shared memory segment size */
#define SHMINFO_MNI		2	/* num shared memory identifiers */
#define SHMINFO_SEG		3	/* max attached shared memory segments per process */

/* The new method for recording interrupt statistics is via three tables
 * which should be defined as follows:
 *
 *		extern volatile unsigned long  system_intr_cnts_level[INTR_MAX_LEVEL];
 *		extern volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
 *      or if NCPUS > 1
 *		extern volatile unsigned long  system_intr_cnts_level[NCPUS][INTR_MAX_LEVEL];
 *		extern volatile unsigned long  system_intr_cnts_type[NCPUS][INTR_TYPE_SIZE];
 *
 *		extern volatile int  *system_intr_cnts_type_transl;
 *
 * When an interrupt occurs, the interrupt dispatcher in /arch/mips/hal/intr.c
 * will increment system_intr_cnts_level[cpu#][level].  This is a count of the 
 * number of interrupts serviced at each interrupt level.  If any other type of
 * interrupt is detected (stray, other, etc), it will increment the
 * system_intr_cntr_type[type] table.
 *
 * If a cpu does not user intr.c as its primary interrupt dispatcher, it will
 * need to do those functions itself.  This can be done either by interrupt
 * level (using the system_intr_cnts_level table) or explicitly by type
 * (using the system_intr_cnts_type table).
 *
 * The pointer *system_intr_cnts_type_transl, points to a table that converts
 * an interrupt level into its associated interrupt type (device, softclock,
 * hardclock, etc).  It is primarily used to allow easy summarization of
 * these counts into the types defined above (INTR_xxx) as done by the
 * routine handler_stats() in /io/common/handler.c.  Each CPU specific 
 * module  (e.g. - kn01.c) must provide this table even if the level table
 * is not used.
 *
 * NOTE:  This whole mechanism has been designed with one design goal:
 *
 *		KEEP THE OVERHEAD IN THE INTERRUPT CODE
 *		       PATH AS LOW AS POSSIBLE.
 *
 *	  As such, this mechanism is more complicated to report the 
 *	  counts, however, it is very easy to record the interrupts.
 */

/*
 * Values for ihs_type  -  interrupt types
 */
#define INTR_MAX_LEVEL 8	/* Maximum num. of interrupt level */
#define INTR_NOSPEC   0x0000
#define INTR_HARDCLK  0x0001
#define INTR_SOFTCLK  0x0002
#define INTR_DEVICE   0x0004
#define INTR_OTHER    0x0008
#define INTR_STRAY    0x8000
#define INTR_DISABLED 0x4000

#define INTR_CLOCK    (INTR_HARDCLK|INTR_SOFTCLK)
#define INTR_NOTCLOCK (~INTR_CLOCK)

/*
 * Values for system_intr_cnts_type and system_intr_cnts_type_tranls tables.
 */

#define INTR_TYPE_NOSPEC   0
#define INTR_TYPE_HARDCLK  1
#define INTR_TYPE_SOFTCLK  2
#define INTR_TYPE_DEVICE   3
#define INTR_TYPE_OTHER    4
#define INTR_TYPE_STRAY    5
#define INTR_TYPE_DISABLED 6
#define INTR_TYPE_PASSIVE_RELEASE 7	/* Filler */

#define INTR_TYPE_SIZE	   8	/* Make it a power of 2 to allow compiler to shift on multiply */

#ifndef   ASSEMBLER
/*
 * Macros for incrementing, looking at the interrupt counter table.
 */

#if NCPUS > 1
#define incr_interrupt_counter_level(level) \
	system_intr_cnts_level[cpu_number()][level]++
#else	/* NCPUS */
#define incr_interrupt_counter_level(level) \
	system_intr_cnts_level[level]++
#endif	/* NCPUS */

#if NCPUS > 1
#define incr_interrupt_counter_type(type) \
	system_intr_cnts_type[cpu_number()][type]++
#else	/* NCPUS */
#define incr_interrupt_counter_type(type) \
	system_intr_cnts_type[type]++
#endif	/* NCPUS */

#if NCPUS > 1
#define examine_interrupt_cnts_level(level) \
	{ \
	   int cpu; \
	   unsigned long count = 0; \
	   for (cpu = 0; cpu < NCPUS; cpu++) { \
	      count += system_intr_cnts_level[cpu][level]; \
           } \
           count \
        }
#else	/* NCPUS */
#define examine_interrupt_cnts_level(level) \
	system_intr_cnts_level[level]
#endif	/* NCPUS */

#if NCPUS > 1
#define examine_interrupt_cnts_type(type) \
	{ \
	   int cpu; \
	   unsigned long count = 0; \
	   for (cpu = 0; cpu < NCPUS; cpu++) { \
	      count += system_intr_cnts_type[cpu][type]; \
           } \
           count \
        }
#else	/* NCPUS */
#define examine_interrupt_cnts_type(type) \
	system_intr_cnts_type[type]
#endif	/* NCPUS */

/*
 *  TBL_FSPARAM data layout
 */

struct tbl_fsparam
{
    long tf_used;		/* free fragments */
    long tf_iused;		/* free inodes */
    long tf_size;		/* total fragments */
    long tf_isize;		/* total inodes */
};


/*
 *  TBL_LOADAVG data layout
 */

struct tbl_loadavg
{
    union {
	    long   l[3];
	    double d[3];
    } tl_avenrun;
    int    tl_lscale;		/* 0 scale when floating point */
    long   tl_mach_factor[3];
};


/*
 *  TBL_INTR data layout
 */

struct tbl_intr
{
	long   	in_devintr;	/* Device interrupts (non-clock) */
	long   	in_context;	/* Context switches */
	long   	in_syscalls;	/* Syscalls */
	long   	in_forks;	/* Forks */
	long   	in_vforks;	/* Vforks */
};


/*
 *  TBL_MODES bit definitions
 */

#define UMODE_P_GID	01	/* - 4.2 parent GID on inode create */
#define UMODE_NOFOLLOW	02	/* - don't follow symbolic links */
#define UMODE_NONICE	04	/* - don't auto-nice long job */



/*
 *	TBL_PROCINFO data layout
 */
#define PI_COMLEN	19	/* length of command string */
struct tbl_procinfo
{
    int		pi_uid;		/* (effective) user ID */
    int		pi_pid;		/* proc ID */
    int		pi_ppid;	/* parent proc ID */
    int		pi_pgrp;	/* proc group ID */
    int		pi_ttyd;	/* controlling terminal number */
    int		pi_status;	/* process status: */
#define PI_EMPTY	0	    /* no process */
#define PI_ACTIVE	1	    /* active process */
#define PI_EXITING	2	    /* exiting */
#define PI_ZOMBIE	3	    /* zombie */
    int		pi_flag;	/* other random flags */
    char	pi_comm[PI_COMLEN+1];
				/* short command name */
    int		pi_ruid;        /* (real) user ID */
    int		pi_svuid;       /* saved (effective) user ID */
    int         pi_rgid;        /* (real) group ID */
    int         pi_svgid;       /* saved (effective) group ID */
    int		pi_session;	/* session ID */
    int         pi_tpgrp;       /* tty pgrp */
    int         pi_tsession;    /* tty session id */
    int         pi_jobc;        /* # procs qualifying pgrp for job control */
    int         pi_cursig;
    sigset_t    pi_sig;         /* signals pending */
    sigset_t    pi_sigmask;     /* current signal mask */
    sigset_t    pi_sigignore;   /* signals being ignored */
    sigset_t    pi_sigcatch;    /* signals being caught by user */
};

/*
 *	TBL_SYSINFO data layout
 */
struct tbl_sysinfo {
        long	si_user;		/* User time */
        long	si_nice;		/* Nice time */
        long	si_sys;			/* System time */
        long	si_idle;		/* Idle time */
        long    si_hz;
        long    si_phz;
	long 	si_boottime;		/* Boot time in seconds */
        long wait; /* Wait time */
#define usr si_user
#define sys si_sys
#define idle si_idle
};

/*
 *	TBL_DKINFO data layout
 */
#define DI_NAMESZ	8
struct tbl_dkinfo {
        int	di_ndrive;
        int	di_busy;
        long	di_time;
        long	di_seek;
        long	di_xfer;
        long	di_wds;
        long	di_wpms;
        int	di_unit;
        char    di_name[DI_NAMESZ+1];
        long di_avque;  /* ave # of outstanding requests */
        long di_avwait; /* ave time (ms) in wait queue */
        long di_avserv; /* ave time (ms) for transfer completion */
};
        
/*
 *	TBL_TTYINFO data layout
 */
struct tbl_ttyinfo {
        long	ti_nin;
        long	ti_nout;
        long	ti_cancc;
        long	ti_rawcc;
        long rcvin;     /* # of receive hardware interrupts (always zero) */
        long xmtin;     /* # of transmit hardware interrupts (always zero) */
        long mdmin;     /* # of modem interrupts (always zero) */
#define rawch ti_nin    /* see above */
#define canch ti_cancc  /* see above */
#define outch ti_nout   /* see above */
};

/*
 *	TBL_SWAPINFO data layout
 */
struct tbl_swapinfo {
	int	flags;
	int	size;
	int	free;
	dev_t	dev;
	ino_t	ino;
	};

/* 
 *	TBL_FILEINFO data layout - file access stats
 */
struct tbl_file {
	long iget;
	long namei;
	long dirblk;
};

/*
 *	 TBL_RUNQ data layout - run queue stats
 */
struct tbl_runq {
	long runque; /* processes in run queue */
	long runocc; /* % of time run queue is occupied */
};

/*
 *	TB__SCALLS data layout - system call stats
 */
struct tbl_scalls {
	long syscall; /* all system calls */
	long sysread; /* read system calls */
	long sysfork; /* fork system calls */
	long syswrite; /* write system calls */
	long sysexec; /* exec system calls */
	ulong readch; /* bytes transferred on read system calls */
	ulong writech; /* bytes transferred on write system calls */
};

/*
 *	TBL_BUFFER data layout - buffer activity
 */
struct tbl_buffer {
	long bread; /* # of physical block reads into system */
	long bwrite; /* # of physical writes from system buffers */
	long lread; /* logical reads from system buffer */
	long lwrite; /* logical writes from system buffer */
	long phread; /* # of physical read requests */
	long phwrite; /* # of physical write requests */
};

/*
 *	TBL_TBLSTATS - file tables stats
 */

struct tbl_tblstats {
	long tb_procu; /* process table entries used */
	long tb_proca; /* process table entries allocated */
	long tb_inodu; /* inode table entries used */
	long tb_inoda; /* inode table entries alocated */
	long tb_fileu; /* file table entries used */
	long tb_filea; /* file table entries allocated */
	long tb_procov; /* proc table overflows */
	long tb_inodov; /* inode table overflows */
	long tb_fileov; /* file table overflows */
	long tb_locku; /* shared memory table entries used */
	long tb_locka; /* shared memory table entries allocated */
};

/*
 *	TBL_KMEM - kernel memory stats
 */

struct tbl_kmem {
	ulong kmem_avail;		/* total kernel memory available */
	ulong kmem_alloc;		/* total kernel memory allocated */
	long kmem_fail;			/* # of kernel memory request failures */
};

/*
 *	TBL_PAGING - paging stats
 */
struct tbl_paging {
	/* sar -g and -p */
	long v_pgpgin;			/* # of pages paged-in */
	long v_sftlock;			/* # of software lock faults */
	long v_pgout;			/* # of page-out requests */
	long v_dfree;			/* # of pages freed */
	long v_scan;			/* # of pages scanned */
	long v_s5ifp;			/* # of s5 inodes taken of freelist */
	/* sar -r */
	long freeswap;			/* # of 512-byte disk blocks avail for page swapping */
};

#define MAPINFO_PFN_MAX 8
enum xlated {UNMAPPED, SEGMENT, MAPPED};
struct tbl_mapinfo {
	vm_offset_t	start_va, end_va;
	vm_prot_t	access;
	enum xlated	mapping;
	unsigned int	pfn[MAPINFO_PFN_MAX];
};

typedef struct {unsigned long vpn:48, pid:16;} * tbl_mapinfo_index;
#define MAPINFO_PID(NDX)	(((tbl_mapinfo_index)&(NDX))->pid)
#define MAPINFO_VPN(NDX)	(((tbl_mapinfo_index)&(NDX))->vpn)

#endif 	/* ASSEMBLER */
#endif	/* _SYS_TABLE_H_ */
