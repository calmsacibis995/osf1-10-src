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
 * @(#)$RCSfile: procfs.h,v $ $Revision: 1.1.13.3 $ (DEC) $Date: 1993/05/22 17:44:27 $
 */

#ifndef _SYS_PROCFS_H_
#define _SYS_PROCFS_H_

#include <sys/signal.h>
#include <sys/siginfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <s5fs/s5param.h>

#ifdef	_KERNEL
#include <kern/macro_help.h>

#else

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (FALSE)

#endif /* _KERNEL */

/* sizes of fields in status and/or ps information structure */

#define CLNAME_SZ	8		/* scheduling class name size */
#define FNAME_SZ	16		/* exec'd file name size */
#define PSARGS_SZ	80		/* ps arguments size */

/* constants for task states */

#define PR_SSLEEP	'S'		/* sleeping: awaiting an event */
#define PR_SRUN		'R'		/* running */
#define PR_SIDL		'I'		/* intermediate state in task
					   creation */
#define PR_SZOMB	'Z'		/* intermediate state in task
					   termination */
#define PR_SSTOP	'T'		/* task is being traced */
#define PR_SDEFAULT	' '		/* default state */

#define NFPREGS		32

struct timestruct {
        long    tv_sec;         /* seconds */
        long    tv_nsec;        /* nanoseconds */
};
typedef struct timestruct timestruct_t;

typedef struct thread * tid_t;	/* thread pointer type */

#define PRC_NREGS 46

struct gregset {
	long	regs[PRC_NREGS];	/* space for general registers */
};

typedef struct gregset gregset_t;


struct fpregset {
	long	regs[PRC_NREGS];	/* space for floating point registers */
};

typedef struct fpregset fpregset_t;

struct tgregset {
	gregset_t pr_regs;	/* space for general registers */
	tid_t	tid;
};

typedef struct tgregset tgregset_t;


struct tfpregset {
	fpregset_t pr_fpregs;	/* space for floating point registers */
	tid_t	tid;
};

typedef struct tfpregset tfpregset_t;

typedef struct tsiginfo {
     siginfo_t pr_siginfo;     /* Signal information to set */
     tid_t    pr_tid;               /* Thread ID */
} tsiginfo_t;

/*
 * These are the type definitions for the set of traced faults, traced system
 * calls, traced signals, general registers and floating point registers that
 * are used by the /proc code.
 */

/* type for set of traced faults first task then thread */

#define FLTSET_SZ	2

typedef struct {
	unsigned long word[FLTSET_SZ];
} fltset_t;

typedef struct tfltset {
     fltset_t pr_flttrace;   /* Fault trace mask */
     tid_t    pr_tid;        /* Thread ID */
} tfltset_t;

/* type for set of traced system calls first task then thread */

#define SYSSET_SZ	9

typedef struct {
	unsigned long word[SYSSET_SZ];
} sysset_t;

#define T_SYSSET_SZ	1

typedef struct {
	unsigned long word[T_SYSSET_SZ];
} t_sysset_t;

typedef struct tsysset {
     t_sysset_t pr_systrace;   /* Sytem call trace mask */
     tid_t    pr_tid;        /* Thread ID */
} tsysset_t;

/* type for set of traced signals for thread level */

typedef struct tsigset {
     sigset_t pr_sigtrace;   /* Signal trace mask */
     tid_t    pr_tid;        /* Thread ID */
} tsigset_t;

/*
 *  This contains the
 *  definitions for the ioctl commands as they are defined for the OSF generic
 *  ioctl routine in bsd/sys_generic.c.  The "com" or command parameter is
 *  passed into the generic ioctl routine as a 32 bit integer in the following
 *  format:
 *
 *	bits 0 - 7	ioctl command number
 *	bits 8 - 15	ioctl group id (use 'F' for /proc)
 *	bits 16 - 28	length for copyin/copyout
 *	bits 29 - 31	direction flag (copyin/copyout/void)
 *
 *  The length is the size of the data that is either copied in from the user
 *  to the kernel or copied out from the kernel to the user.
 *
 *  If the direction flag is set to copyin or copyout, user data is passed to
 *  or from the kernel respectively.  If no data is passed to or returned from
 *  the kernel, the direction is set to void.  The _IOR macro is for copyin, the
 *  _IOW macro is for copyout and the _IO macro is for void.  The arguments for
 *  the _IOR and _IOW macros are the group id, the ioctl command number and the
 *  data that is passed.  The arguments to the _IO macro are the group id and
 *  the ioctl number.  For more information see the ioctl.h header file where
 *  these macros are defined.
 *
 *  This header file also contains /proc data structures and definitions for all
 *  the flags used in the flag fields in the data structures.
 */

/* mask to get the /proc ioctl command number */

#define PR_CMD_MASK	0xff

/* codes for /proc ioctl system call */

/* ioctls for task level */

#define PIOCCRED     _IOR('F',1,struct prcred)    /* get credentials */
#define PIOCGETPR    _IOR('F',2,struct proc)      /* get proc structure */
#define PIOCGETU     _IOR('F',3,struct user)      /* get user structure */
#define PIOCGROUPS   _IOR('F',4,gid_t)            /* get group ids */
#define PIOCPSINFO   _IOR('F',5,struct prpsinfo)  /* get ps info */
#define PIOCNICE     _IOW('F',6,int)              /* set nice priority */
#define PIOCOPENM    _IOR('F',7,caddr_t)          /* get mapped object */
#define PIOCRUN      _IOW('F',8,struct prrun)     /* make task runable */
#define PIOCSTATUS   _IOR('F',9,struct prstatus)  /* get task status */
#define PIOCSTOP     _IOR('F',10,struct prstatus) /* stop a task */
#define PIOCWSTOP    _IOR('F',11,struct prstatus) /* stop a task w/wait*/
#define PIOCGTRACE   _IOR('F',12,sigset_t)        /* get traced signals */
#define PIOCSTRACE   _IOW('F',13,sigset_t)        /* set traced signals */
#define PIOCSSIG     _IOW('F',14,struct siginfo)  /* set sig to siginfo */
#define PIOCKILL     _IOW('F',15,int)             /* send sig to task */
#define PIOCUNKILL   _IOW('F',16,int)             /* delete pending sig */
#define PIOCGHOLD    _IOR('F',17,sigset_t)        /* get held signals */
#define PIOCSHOLD    _IOW('F',18,sigset_t)        /* set held signals */
#define PIOCCFAULT   _IO('F',19)                  /* clear current flt */
#define PIOCGFAULT   _IOR('F',20,fltset_t)        /* get traced faults */
#define PIOCSFAULT   _IOW('F',21,fltset_t)        /* set traced faults */
#define PIOCGENTRY   _IOR('F',22,sysset_t)        /* get traced syscall entry */
#define PIOCSENTRY   _IOW('F',23,sysset_t)        /* set traced syscall entry */
#define PIOCGEXIT    _IOR('F',24,sysset_t)        /* get traced syscall exit */
#define PIOCSEXIT    _IOW('F',25,sysset_t)        /* set traced syscall exit */
#define PIOCRFORK    _IO('F',26)                  /* reset (turn off)
                                                     inherit-on-fork */
#define PIOCSFORK    _IO('F',27)                  /* set inherit-on-fork */
#define PIOCRRLC     _IO('F',28)                  /* reset (turn off)
                                                     run-on-last-close */
#define PIOCSRLC     _IO('F',29)                  /* set run-on-last-close */
#define PIOCGFPREG   _IOR('F',30,fpregset_t)      /* get floating point regs */
#define PIOCSFPREG   _IOW('F',31,fpregset_t)      /* set floating point regs */
#define PIOCGREG     _IOR('F',32,gregset_t)       /* get general regs */
#define PIOCSREG     _IOW('F',33,gregset_t)       /* set general regs */
#define PIOCMAXSIG   _IOR('F',34,int)             /* get max signals */
#define PIOCACTION   _IOR('F',35,struct sigaction)   /* get sig actions */
#define PIOCNMAP     _IOR('F',36,int)             /* get # mem mappings */
#define PIOCMAP      _IOR('F',37,struct prmap)    /* get mem mappings */
#define PIOCGETTK    _IOR('F',38,struct task)     /* get the task struct */
#define PIOCGSPCACT  _IOR('F',39,int)             /* get special action value */
#define PIOCSSPCACT  _IOW('F',40,int)             /* set special action value */
#define PIOCNTHR     _IOR('F',41,int)             /* get thread count */
#define PIOCTLIST    _IOR('F',42,tid_t)           /* get thread ids */
#define PIOCGETUTK   _IOR('F',43,struct utask)    /* get the utask struct */

/* ioctls for one or more threads */

#define PIOCTRUN     _IOWR('F',45,struct prrun)   /* make thread runable */
#define PIOCTGETTH   _IOWR('F',46,struct thread)  /* get the thread struct */
#define PIOCTSTATUS  _IOWR('F',47,struct prstatus)   /* get thread status */
#define PIOCTSTOP    _IOWR('F',48,struct prstatus)   /* stop a thread */
#define PIOCTGTRACE  _IOWR('F',49,tsigset_t)      /* get traced signals */
#define PIOCTSTRACE  _IOWR('F',50,tsigset_t)      /* set traced signals */
#define PIOCTSSIG    _IOWR('F',51,tsigset_t)      /* set sig to siginfo */
#define PIOCTKILL    _IOWR('F',52,tid_t)          /* send sig to thread */
#define PIOCTUNKILL  _IOWR('F',53,tid_t)          /* delete pending sig */
#define PIOCTCFAULT  _IOWR('F',54,tfltset_t)      /* clear faults */
#define PIOCTGFAULT  _IOWR('F',55,tfltset_t)      /* get traced faults */
#define PIOCTSFAULT  _IOWR('F',56,tfltset_t)      /* set traced faults */
#define PIOCTGFPREG  _IOWR('F',57,tfpregset_t)    /* get floating point regs */
#define PIOCTSFPREG  _IOWR('F',58,tfpregset_t)    /* set floating point regs */
#define PIOCTGREG    _IOWR('F',62,tgregset_t)     /* get general regs */
#define PIOCTSREG    _IOWR('F',63,tgregset_t)     /* set general regs */
#define PIOCTACTION  _IOWR('F',64,struct sigaction)  /* get sig actions */
#define PIOCTTERM    _IOWR('F',66,tid_t)          /* terminate a thread */
#define PIOCTABRUN   _IOWR('F',69,prrun_t)        /* run all but threads
                                                       in thread list */
#define PIOCTGETUTH  _IOWR('F',70,struct uthread) /* get the uthread struct */

/*
 * The prstatus data structure contains status information that is passed to the
 * user from the kernel when the user invokes the PIOCSTATUS command.  This
 * structure contains status information for the task or thread that is being
 * traced.
 */

/* values for pr_flags in /proc status structure */

#define PR_STOPPED	0x0001L	/* task/thread is stopped */
#define PR_ISTOP	0x0002L	/* task/thread stopped on event of interest */
#define PR_DSTOP	0x0004L	/* task/thread has stop directive in effect */
#define PR_ASLEEP	0x0008L	/* task/thread is asleep within a system call */
#define PR_FORK		0x0010L	/* task/thread has inherit-on-fork flag set */
#define PR_RLC		0x0020L	/* task/thread has run-on-last-close flag set */
#define PR_PTRACE	0x0040L	/* task/thread is being traced by ptrace */
#define PR_PCINVAL	0x0080L	/* program counter contains invalid address */
#define PR_ISSYS	0x0100L	/* task/thread is a system task/thread */

/* values for pr_why in /proc status structure */

#define PR_REQUESTED	1	/* task/thread stopped via PIOCSTOP */
#define PR_SIGNALLED	2	/* task/thread stopped on receipt of signal */
#define PR_FAULTED	3	/* task/thread stopped on receipt of fault */
#define PR_SYSENTRY	4	/* task/thread stopped on syscall entry */
#define PR_SYSEXIT	5	/* task/thread stopped on syscall exit */
#define PR_JOBCONTROL	6	/* task/thread stopped due to default action
				   of job control signal */
#define PR_DEAD		0xDEAD	/* task/thread stopped in exit code */

/* /proc status structure */

struct prstatus {
	long	pr_flags;		/* status flags */
	short	pr_why;			/* why task/thread is stopped */
	long	pr_what;		/* what stopped the task/thread */
	siginfo_t pr_info;		/* signal information struct */
	short	pr_cursig;		/* current signal */
	sigset_t pr_sigpend;		/* set of pending signals */
	sigset_t pr_sighold;		/* set of held signals */
	stack_t	pr_altstack;		/* alternate signal stack */
	struct  sigaction pr_action;	/* current signal action */
	pid_t	pr_pid;			/* process id */
	pid_t	pr_ppid;		/* parent process id */
	gid_t	pr_pgrp;		/* group id */
	pid_t	pr_sid;			/* session id */
	timestruct_t pr_utime;		/* user cpu time */
	timestruct_t pr_stime;		/* system cpu time */
	timestruct_t pr_cutime;		/* sum of childrens user times */
	timestruct_t pr_cstime;		/* sum of childrens system times */
	char	pr_clname[CLNAME_SZ];	/* scheduling class name */
	long	pr_filler[20];		/* for future expansion */
	long	pr_instr;		/* current instruction */
	gregset_t pr_reg;		/* general registers */
	u_long	pr_subcode;		/* breakpoint (and other) subcode */
	long	pr_nthreads;		/* number of threads */
	struct vnode * pr_exvp;		/* vp for executable on disk */
	tid_t	pr_tid;			/* thread id */
					/* NOTE: pr_tid must be the last
					 * element in this structure. Add any
					 * new elements before it.
					 */
};

typedef struct prstatus prstatus_t;

/*
 * The prrun data structure contains information that is passed from the user to
 * the kernel when the user invokes the PIOCRUN command.  It allows the user to
 * specify certain actions to be taken before a stopped task or thread is run.
 * The actions are performed and the task or thread is resumed.
 */

/* values for pr_flags in run structure */

#define	PRCSIG		0x0001L		/* clear current signal */
#define PRCFAULT	0x0002L		/* clear current fault */
#define PRSTRACE	0x0004L		/* set traced signal set to pr_trace */
#define PRSHOLD		0x0008L		/* set held signal set to pr_hold */
#define PRSFAULT	0x0010L		/* set traced fault set to pr_fault */
#define PRSVADDR	0x0020L		/* set resume address to pr_vaddr */
#define PRSTEP		0x0040L		/* execute in single-step mode */
#define PRSTOP		0x0080L		/* set the demand stop bit */
#define PRSABORT	0x0100L		/* abort system call if PR_SYSENTRY stop
					   or PR_ASLEEP state */
/* process run structure */

struct prrun {
	long	pr_flags;		/* flags */
	sigset_t pr_trace;		/* set of traced signals */
	sigset_t pr_sighold;		/* set of held signals */
	fltset_t pr_fault;		/* set of faults traced */
	caddr_t	pr_vaddr;		/* resume virtual address */
	long	filler[8];		/* for future expansion */
	tid_t	pr_tid;			/* id for PIOCTRUN, ignored by PIOCRUN*/
					/* NOTE: pr_tid must be the last
					 * element in this structure. Add any
					 * new elements before it.
					 */
};

typedef struct prrun prrun_t;

/* The thread specific IOCTLs need a capacity to pass lists of threads and
 * structures of interest.  The prthreads structure is defined to aid this 
 */
typedef struct prthreads {
	long     pr_count;          /* number of threads in list */
	char     pr_data[1];           /* thread ioctl data */
} prthreads_t;

/* /proc thread run-all-but structure
 *
 * This structure contains a count of the threads listed
 * as not to be started, a flag as to whether the data in the enclosed
 * prrun structure included is valid, and a list of thread IDs.
 * This gives all the information to modify and control the thread in the task
 * that are not listed by the caller.
 */


struct prabrun {
     long     pr_count;        /* Number of threads listed */
     long     pr_flags;        /* run data is valid flag */
     prrun_t  pr_run;          /* Run data to apply to started threads */
     tid_t    *pr_tid;         /* pointer to list of threads _NOT_ to run */
};

typedef struct prabrun prabrun_t;

/*
 * The prpsinfo data structure contains information that is passed to the user
 * from the kernel when the user invokes the PIOCPSINFO command.  It contains
 * the information reported by the ps user command.  The information stored in
 * this structure is compatible with the System V Release 4 ps command.
 */


/* ps information structure */

struct prpsinfo {
	char	pr_state;		/* task/thread state */
	char	pr_sname;		/* pr_state in printable char form */
 	char	pr_zomb;		/* task/thread terminated but not waited
					   for */
	char	pr_nice;		/* nice value */
	u_long	pr_flag;		/* process flags */
	uid_t	pr_uid;			/* user id */
	gid_t	pr_gid;			/* group id */
	pid_t	pr_pid;			/* process id */
	pid_t	pr_ppid;		/* parent process id */
	pid_t	pr_pgrp;		/* process group leader pid */
	pid_t	pr_sid;			/* session id */
	caddr_t	pr_addr;		/* physical address */
	long	pr_size;		/* image size in pages */
	long	pr_rssize;		/* resident set size in pages */
	caddr_t	pr_wchan;		/* sleeping task/thread wait address */
	timestruct_t pr_start;		/* start time */
	timestruct_t pr_time;		/* usr+sys cpu time */
	long	pr_pri;			/* task/thread priority */
	char	pr_oldpri;		/* old task/thread priority; pre-SVR4 */
	char	pr_cpu;			/* cpu usage; pre-SVR4 */
	dev_t	pr_ttydev;		/* controlling tty dev(PRNODEV if none*/
	char	pr_clname[CLNAME_SZ];	/* scheduling class name */
	char	pr_fname[FNAME_SZ];	/* last part of exec'd path name */
	char	pr_psargs[PSARGS_SZ];	/* arg list initial characters */
	long	pr_filler[20];		/* for future use */
};

typedef struct prpsinfo prpsinfo_t;

/*
 * The prmap data structure contains information that is passed to the user from
 * the kernel when the user invokes the PIOCMAP command. This structure contains
 * the memory map information for the traced task or thread.
 */

/* values for pr_mflags in memory mapping structure */

#define MA_READ		0x0001L		/* readable by traced task/thread */
#define MA_WRITE	0x0002L		/* writable by traced task/thread */
#define MA_EXEC		0x0004L		/* executable by traced task/thread */
#define MA_SHARED	0x0008L		/* changes shared by mapped object */
#define MA_BREAK	0x0010L		/* mapping grown by brk syscall */
#define MA_STACK	0x0020L		/* mapping grown by stack faults */

/* memory mapping structure */

struct prmap {
	caddr_t	pr_vaddr;		/* base virtual address */
	u_long	pr_size;		/* mapping size in bytes */
	off_t	pr_off;			/* offset into mapped object */
	long	pr_mflags;		/* protection and attribute flags */
	long	filler[4];		/* for future use */
};

typedef struct prmap prmap_t;

/*
 * The prcred data structure contains information that is passed to the user
 * from the kernel when the user invokes the PIOCCRED command.  This structure
 * contains the credentials of the traced task or thread.
 */

/* credentials structure */

struct prcred {
	uid_t	pr_euid;		/* effective user id */
	uid_t	pr_ruid;		/* real user id */
	uid_t	pr_suid;		/* user id saved from exec */
	gid_t	pr_egid;		/* effective group id */
	gid_t	pr_rgid;		/* real group id */
	gid_t	pr_sgid;		/* group id saved from exec */
	u_int	pr_ngroups;		/* number of supplementary groups */
};

typedef struct prcred prcred_t;

/*
 * macros for manipulating signal, fault and system call trace flags
 * it is assumed that the size of a word is 32 bits
 * sp is a pointer to sigset_t, fltset_t, or sysset_t
 * flag is a number corresponding to a trace flag in *sp.
 */

#ifdef __alpha
#define PR_SHFT 6
#define PR_MSK 0x3f
#else
#define PR_SHFT 5
#define PR_MSK 0x1f
#endif

/* turn on all trace flags in set */
#define	prfillset(sp) MACRO_BEGIN \
	register long _i_ = sizeof(*(sp))/sizeof(long); \
		while(_i_) \
			((long*)(sp))[--_i_] = -1; MACRO_END

/* turn off all trace flags in set */
#define	premptyset(sp) MACRO_BEGIN \
	register long _i_ = sizeof(*(sp))/sizeof(long); \
		while(_i_) \
			((long*)(sp))[--_i_] = 0L; MACRO_END
/* turn on specified trace flag in set */
#define	praddset(sp, flag) \
	(((u_long)((flag)) < (sizeof(long)*NBBY) * sizeof((*sp))/sizeof(long)) ? \
	(((long*)(sp))[((flag))>>PR_SHFT] |= (1L<<(((flag))&PR_MSK))) : 0)

/* turn off specified trace flag in set */
#define	prdelset(sp, flag) \
	(((u_long)((flag)) < (sizeof(long)*NBBY) * sizeof((*sp))/sizeof(long)) ? \
	(((long*)(sp))[((flag))>>PR_SHFT] &= ~(1L<<(((flag))&PR_MSK))) : 0)

/* query: 0 if trace flag is turned off in set
 *        1 iff trace flag is turned on in set
 */
#define	prismember(sp, flag) \
	(((u_long)((flag)) < (sizeof(long)*NBBY) * sizeof((*sp))/sizeof(long)) \
	&& (((long*)(sp))[((flag))>>PR_SHFT] & (1L<<(((flag))&PR_MSK))))


/* Structure containing /proc information for each task and
 * included as part of the task structure.
 * Note: originally, this structure was used for both task and thread.  But,
 * in order to hold down kernel size, the t_procfs structure below was created
 * for threads, and is included in the thread structure.
 */

struct procfs {
	int	pr_qflags;		/* flags for doing quick checks */
	int	pr_ssig;		/* signal forced by PIOCSSIG */
	short	pr_why;			/* why task/thread is stopped */
	long	pr_flags;		/* /proc status flags */
	long	pr_curflt;		/* current fault */
	long	pr_what;		/* what stopped the task/thread */
	sigset_t pr_sigtrace;		/* signals traced by /proc */
	fltset_t pr_flttrace;		/* faults traced by /proc */
	sysset_t pr_sysenter;		/* syscalls traced on entry by /proc */
	sysset_t pr_sysexit;		/* syscalls traced on exit by /proc */
	int	pr_nmap;		/* number of map entries (PIOCMAP) */
	uid_t	pr_uid;			/* user id of exec'd file */
	gid_t	pr_gid;			/* group id of exec'd file */
	int	pr_protect;		/* permissions of exec'd file */
	int	pr_subcode;		/* breakpoint (and other) subcode */
	struct vnode *pr_exvp;		/* vnode of executing image */
	tid_t	pr_tid;			/* thread id */
					/* NOTE: pr_tid must be the last
					 * element in this structure. Add any
					 * new elements before it.
					 */
};

typedef struct procfs procfs_t;


/* Structure containing /proc information for each thread and
 * included as part of the thread structure.
 */

struct t_procfs {
	int	pr_qflags;		/* flags for doing quick checks */
	short	pr_why;			/* why task/thread is stopped */
	long	pr_flags;		/* /proc status flags */
	long	pr_curflt;		/* current fault */
	long	pr_what;		/* what stopped the task/thread */
	sigset_t pr_sigtrace;		/* signals traced by /proc */
	fltset_t pr_flttrace;		/* faults traced by /proc */
	t_sysset_t pr_sysenter;		/* syscalls traced on entry by /proc */
	t_sysset_t pr_sysexit;		/* syscalls traced on exit by /proc */
	int	pr_subcode;		/* breakpoint (and other) subcode */
	tid_t	pr_tid;			/* thread id */
					/* NOTE: pr_tid must be the last
					 * element in this structure. Add any
					 * new elements before it.
					 */
};

typedef struct t_procfs t_procfs_t;

/* values for pr_qflags in the procfs structure */

#define PRFS_SCENTER	0x00000001	/* tracing system call on entry */
#define PRFS_SCEXIT	0x00000002	/* tracing system call on exit */
#define PRFS_FAULT	0x00000004	/* tracing fault */
#define PRFS_SIGNAL	0x00000008	/* tracing fault */
#define PRFS_ABORT	0x00000010	/* abort a system call */
#define PRFS_OPEN	0x00000020	/* task/thread is opened by /proc */
#define PRFS_FDINVLD	0x00000040	/* file descriptor invalid */
#define PRFS_EXECED	0x00000080	/* process was exec'd */
#define PRFS_TRACING	0x00000100	/* task open for write -can be traced */
#define PRFS_STOPEXEC	0x00000400	/* stop on exec */
#define PRFS_STOPTERM	0x00000800	/* stop on task exit */
#define PRFS_STOPTCR	0x00001000	/* stop on thread creation */
#define PRFS_STOPTTERM	0x00002000	/* stop on thread exit */

#define PRFS_SPCACTMASK \
 (PRFS_STOPEXEC | PRFS_STOPTERM | PRFS_STOPTCR | PRFS_STOPTTERM)


#define VNTOPN(vn) ((struct procnode *)(vn)->v_data)
#define PNTOVN(pn) ((struct vnode *)(pn)->prc_vp)
#define PROCFS_FHTOVP(fh) ((struct vnode *)(fh)->fid_data)

/* struct used in file handle to vnode translations, overlays the fid
 * struct in mount.h
 */

struct pfid
{
	u_short	pfid_len;	/* length of this struct */
	u_short	pfid_pad;	/* ensure vnode ptr is longword aligned */
	struct vnode *pfid_vnode; /* vnode that this handle refers to */
	long	pfid_gen;	/* generation number, always 0 */
};


/* "procnode" struct which is included in the vnode at v_data, just like
 * inode for the ufs file system
 */

struct procnode
{
	int	prc_flags;	/* status flags */
	int	prc_pid;	/* pid == file name in /proc */
	int	prc_index;	/* index into the proc table for this pid */
	pid_t	prc_excl;	/* pid if an exclusive open for write was done*/
	int	prc_exitval;	/* exit status of "traced" process if it exits*/
	struct timeval prc_time; /*proc creation time, detects "stale" entries*/
	struct vnode *prc_vp;	/* pointer to the vnode that we are in */
};

/* flag defines for procnode.prc_flags */
#define	PRC_FDINVLD	0x00000001	/* all fd's that refer to this vnode
					 * are invalid */


#endif /* _SYS_PROCFS_H_ */
