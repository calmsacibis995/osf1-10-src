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
 *	@(#)$RCSfile: signal.h,v $ $Revision: 4.3.13.14 $ (DEC) $Date: 1994/01/31 23:58:09 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * COMPONENT_NAME: SYSPROC - signal.h
 *                                                                    
 * ORIGIN: 27
 *
 * Copyright International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#ifndef _SYS_SIGNAL_H_
#define _SYS_SIGNAL_H_

#include <standards.h>

/* add this for early ALPHA compiler skew problem */
#if defined(LANGUAGE_C) && !defined(__LANGUAGE_C__)
#define __LANGUAGE_C__
#endif

/*
 *
 *      The ANSI standard requires that certain values be in signal.h.
 *	The ANSI standard allows additional signals and pointers to 

 *	undeclarable functions with macro definitions beginning with
 * 	the letters SIG or SIG_ and an upper case letter.
 *      However, it also requires that if _ANSI_C_SOURCE is defined then 
 *      no other function definitions are present
 *
 *      This header includes all the ANSI required entries.  In addition
 *      other entries for the OSF system are included.
 *
 */

#ifdef _ANSI_C_SOURCE
# if !defined(_KERNEL) && defined(__LANGUAGE_C__)
#  if defined (_NO_PROTO)
extern void (*signal())();
#  else   /* ~_NO_PROTO */
#if defined(__cplusplus)
extern "C" {
#endif
extern void (*signal(int, void (*)(int)))(int);
#if defined (__cplusplus)
}
#endif
#  endif  /* _NO_PROTO */

#  ifdef _NO_PROTO
extern int raise();
#  else /* ~ _NO_PROTO */
#if defined(__cplusplus)
extern "C" {
#endif
extern int raise( int );
#if defined (__cplusplus)
}
#endif
#  endif /* _NO_PROTO */

typedef int sig_atomic_t; /* accessable as an atomic entity (ANSI) */

# endif /* !_KERNEL && __LANGUAGE_C__ */

#endif /* _ANSI_C_SOURCE */

/*
 * valid signal values: all undefined values are reserved for future use 
 * note: POSIX requires a value of 0 to be used as the null signal in kill()
 */
#define	SIGHUP	   1	/* hangup, generated when terminal disconnects */
#define	SIGINT	   2	/* interrupt, generated from terminal special char */
#define	SIGQUIT	   3	/* (*) quit, generated from terminal special char */
#define	SIGILL	   4	/* (*) illegal instruction (not reset when caught)*/
#define	SIGTRAP	   5	/* (*) trace trap (not reset when caught) */
#define	SIGABRT    6	/* (*) abort process */
#define SIGEMT	   7	/* EMT instruction */
#define	SIGFPE	   8	/* (*) floating point exception */
#define	SIGKILL	   9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	  10	/* (*) bus error (specification exception) */
#define	SIGSEGV	  11	/* (*) segmentation violation */
#define	SIGSYS	  12	/* (*) bad argument to system call */
#define	SIGPIPE	  13	/* write on a pipe with no one to read it */
#define	SIGALRM	  14	/* alarm clock timeout */
#define	SIGTERM	  15	/* software termination signal */
#define	SIGURG 	  16	/* (+) urgent contition on I/O channel */
#define	SIGSTOP	  17	/* (@) stop (cannot be caught or ignored) */
#define	SIGTSTP	  18	/* (@) interactive stop */
#define	SIGCONT	  19	/* (!) continue (cannot be caught or ignored) */
#define SIGCHLD   20	/* (+) sent to parent on child stop or exit */
#define SIGTTIN   21	/* (@) background read attempted from control terminal*/
#define SIGTTOU   22	/* (@) background write attempted to control terminal */
#define SIGIO	  23	/* (+) I/O possible, or completed */
#define SIGXCPU	  24	/* cpu time limit exceeded (see setrlimit()) */
#define SIGXFSZ	  25	/* file size limit exceeded (see setrlimit()) */
#define SIGVTALRM 26	/* virtual time alarm (see setitimer) */
#define SIGPROF   27	/* profiling time alarm (see setitimer) */
#define SIGWINCH  28	/* (+) window size changed */
#define SIGINFO   29    /* information request */
#define SIGUSR1   30	/* user defined signal 1 */
#define SIGUSR2   31	/* user defined signal 2 */

/*
 * additional signal names supplied for compatibility, only 
 */
#define SIGIOINT SIGURG	/* printer to backend error signal */
#define SIGAIO	SIGIO	/* base lan i/o */
#define SIGPTY  SIGIO	/* pty i/o */
#define	SIGPOLL	SIGIO	/* STREAMS version of this signal */
#define SIGIOT  SIGABRT /* abort (terminate) process */ 
#define	SIGLOST	SIGIOT	/* old BSD signal ?? */
#define SIGPWR  SIGINFO /* Power Fail/Restart -- SVID3/SVR4 */
/*
 * valid signal action values; other values => pointer to handler function 
 */
#ifndef	_NO_PROTO
#define SIG_ERR		(void (*)(int))-1
#define	SIG_DFL		(void (*)(int))0
#define	SIG_IGN		(void (*)(int))1
#define SIG_HOLD        (void (*)(int))2        /* not valid as argument
                                                   to sigaction or sigvec */
#define SIG_CATCH       (void (*)(int))3        /* not valid as argument
                                                   to sigaction or sigvec */
#else	/* _NO_PROTO */
#define SIG_ERR		(void (*)())-1
#define	SIG_DFL		(void (*)())0
#define	SIG_IGN		(void (*)())1
#define SIG_HOLD        (void (*)())2           /* not valid as argument
                                                   to sigaction or sigvec */
#define SIG_CATCH       (void (*)())3           /* not valid as argument
                                                   to sigaction or sigvec */
#endif	/* _NO_PROTO */

/*
 *   The following are values that have historically been in signal.h.
 *
 *   They are a part of the POSIX defined signal.h and therefore are
 *   included when _POSIX_SOURCE is defined.
 *
 */

#ifdef _POSIX_SOURCE

/*
 * values of "how" argument to sigprocmask() call
 */
#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

#include <sys/types.h>	/* Cannot be in ANSI - name space pollution */

/*
 * valid flag define for sa_flag field of sigaction structure - POSIX
 */
#define SA_NOCLDSTOP	0x00000004	/* do not set SIGCHLD for child stops*/


#ifdef __LANGUAGE_C__
/*
 * sigaction structure used in sigaction() system call 
 * The order of the fields in this structure must match those in
 * the sigvec structure (below).
 */
struct sigaction {
#ifdef _NO_PROTO
	void	(*sa_handler)();	/* signal handler, or action value */
#else	/* _NO_PROTO */
	void	(*sa_handler)(int);	/* signal handler, or action value */
#endif	/* _NO_PROTO */
	sigset_t sa_mask;		/* signals to block while in handler */
	int	sa_flags;		/* signal action flags */
};

#if defined(_POSIX_4SOURCE) || defined(_KERNEL)
/*
 * the following hackery is used till signals are ||ized
 */
#if NCPUS == 1
#define pgsignal_tty(pg, sig, checktty, doself) pgsignal(pg, sig, checktty)
#define psignal_tty psignal
#endif

/*
 * sigval union and sigevent structure needed by Async I/O and Timer routines.
 * Defines values passed to signal handlers on I/O completion or timer
 * expiration.
 */
typedef union sigval {
	int 	sival_int;
	void	*sival_ptr;
} sigval_t;

/*
 * Define some nonstandard shortcuts to reference the union fields...
 */
#define sigev_int sigev_value.sival_int
#define sigev_ptr sigev_value.sival_ptr

typedef struct sigevent {
	union sigval	sigev_value;	/* application-defined value */
	int		sigev_signo;	/* signal to raise */
} sigevent_t;

/*
 * DEFINEs for compatibility with P1003.4/D10 struct sigevent
 */
#ifdef POSIX_4D10
#define sevt_signo sigev_signo
#define sevt_value sigev_value.sival_ptr
#endif
#endif	/* _POSIX_4SOURCE */

#ifndef _KERNEL
#ifdef _NO_PROTO
extern int kill();
extern int sigaction();
extern int sigprocmask();
extern int sigsuspend();
extern int sigemptyset();
extern int sigfillset();
extern int sigaddset();
extern int sigdelset();
extern int sigismember();
extern int sigpending();

#ifdef _POSIX_4SOURCE
extern int sigwait();
#endif

#else	/* _NO_PROTO */

/*
 * function prototypes for signal functions 
 */
/* system calls */
#if defined(__cplusplus)
extern "C" {
#endif
extern int kill(pid_t , int ); 
extern int sigaction(int , const struct sigaction *, struct sigaction *);
extern int sigprocmask(int , const sigset_t *, sigset_t *);
extern int sigsuspend(const sigset_t *);
/* library routines */
extern int sigemptyset(sigset_t *);
extern int sigfillset(sigset_t *);
extern int sigaddset(sigset_t *, int );
extern int sigdelset(sigset_t *, int );
extern int sigismember(const sigset_t *, int );
extern int sigpending(sigset_t *);

#ifdef _POSIX_4SOURCE
extern int sigwait(sigset_t *);
#endif

#if defined(__cplusplus)
}
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#ifndef lint
#ifdef _KERNEL
/*
 * These are library routines.  Let the kernel use these until they get
 * their POSIX story straight.
 */
#define sigemptyset(set)        { *(set) = (sigset_t)0; }
#define sigfillset(set)         { *(set) = ~(sigset_t)0; }
#define sigaddset(set, signo)   ( *(set) |= 1L << ((signo) - 1), 0)
#define sigdelset(set, signo)   ( *(set) &= ~(1L << ((signo) - 1)), 0)
#define sigismember(set, signo) ( (*(set) & (1L << ((signo) - 1))) != (sigset_t)0)
#endif /* _KERNEL */
#endif /* lint */

#endif 	/* __LANGUAGE_C__ */

#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE


/*---------------------------------------------------------------*/
/* include file needed for Floating Point emulation. We can take */
/* this out when we get real hardware.  			 */
/*---------------------------------------------------------------*/
#ifdef _IBMRT
#include <ieeetrap.h>
#endif /* _IBMRT */

/*
 * Macro for converting signal number to a mask suitable for
 * sigblock().
 */
#define sigmask(m)	(1L << ((m)-1))

#define sigcantmask     (sigmask(SIGKILL)|sigmask(SIGSTOP))

/*
 * sigvec structure used in sigvec compatibility interface.
 * The order of the fields in this structure must match those in
 * the sigaction structure (above).
 */
#ifdef __LANGUAGE_C__
struct	sigvec {
#ifdef _NO_PROTO
	void	(*sv_handler)();	/* signal handler */
#else	/* _NO_PROTO */
	void	(*sv_handler)(int);	/* signal handler */
#endif	/* _NO_PROTO */
	int     sv_mask;        /* signal mask to apply */
	int     sv_flags;    
};                           
#define sv_onstack sv_flags     /* isn't compatibility wonderful! */

/*
 * function prototypes for signal functions 
 */

#ifndef _KERNEL
#ifdef _NO_PROTO
extern int sigvec();
#else	/* _NO_PROTO */
#if defined(__cplusplus)
extern "C" {
#endif
extern int sigvec(int, struct sigvec *, struct sigvec *);
#if defined(__cplusplus)
}
#endif
#endif	/* _NO_PROTO */
#endif	/* !_KERNEL */
#endif	/* __LANGUAGE_C__ */


/*
 *      signals delivered on a per-thread basis.
 */
#ifdef _KERNEL
#define threadmask                                              \
        (sigmask(SIGILL)|sigmask(SIGTRAP)|sigmask(SIGIOT)|      \
         sigmask(SIGEMT)|sigmask(SIGFPE)|sigmask(SIGBUS)|       \
         sigmask(SIGSEGV)|sigmask(SIGSYS)|sigmask(SIGPIPE))

#define thread_signal_disposition(signo) (sigmask(signo)&threadmask)

#define signal_disposition(signo) (*((thread_signal_disposition(signo) ? u.u_tsignal : u.u_signal)+signo))

/*
 * get signal action for process and signal; currently only for current process
 */
#define SIGACTION(p, sig)       (signal_disposition(sig)))

#endif /* _KERNEL */

/*
 * sigstack structure used in sigstack() system call 
 */

#ifdef __LANGUAGE_C__
struct  sigstack {
	char	*ss_sp;                 /* signal stack pointer */
        int     ss_onstack;             /* current status */
};

/*
 * SVR4 version of sigstack, used in sigaltstack() system call
 */
typedef struct  sigaltstack {
        caddr_t	ss_sp;			/* signal stack pointer */
        int     ss_flags;		/* current status */
        size_t	ss_size;		/* size of stack - 0 if unknown */
} stack_t;

/*
 * sigaltstack() control flags
 */
#define	SS_ONSTACK	0x00000001	/* use signal stack */
#define SS_DISABLE	0x00000002	/* disable (don't use) signal stack */
#define SS_NOMASK	0x00000004 	/* no mask restore on sigreturn */
#define SS_STACKMAGIC	0xfafababe00000000
					/* stack data restore on sigreturn */
#define _SSTACKFLAGS	(SS_ONSTACK|SS_DISABLE)

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 */
struct  sigcontext {
#ifdef  __alpha
	/*
	 *  Backward compatibility -- correlates with user space
	 *  notion of layout.
	 */
	long    sc_onstack;		/* sigstack state to restore */
	long    sc_mask;		/* signal mask to restore */
#else
        int     sc_onstack;             /* sigstack state to restore */
        int     sc_mask;                /* signal mask to restore */
#endif /* __alpha */
#ifdef  vax
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif  /* vax */
#if     defined(mc68000) || defined(__mc68000)
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif  /* mc68000 */
#ifdef  multimax
        int     sc_r0;                  /* r0 to restore */
        int     sc_r1;                  /* r1 to restore */
        int     sc_r2;                  /* r2 to restore */
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif  /* multimax */
#ifdef  balance
        int     sc_sp;                  /* sp to restore */
        int     sc_modpsr;              /* mod/psr to restore */
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif  /* balance */
#ifdef  ibmrt
#define MAXSIGREGS      33      /* (INFO-R0+1) num regs sent to sighndlr */
        char    *sc_floatsave;          /* -> (struct floatsave *) */
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_iar;                 /* ibmrt iar equivalent */
        int     sc_icscs;               /* ibmrt psl equivalent */
#define sc_pc   sc_iar
#define sc_ps   sc_icscs
        int     sc_saveiar;             /* saved copy of IAR for RTFL */
        int     sc_regs[MAXSIGREGS];    /* must be after sc_icscs */
#define sc_flags        sc_onstack
#define SC_ONSTACK      SV_ONSTACK      /* sc_flags: on signal stack */
#define SC_FLOATSAVE    0x8000          /* sc_flags: floating pt saved */
#define SC_EXCEPTION    0x4000          /* sc_flags: excpt packets saved */
#define SC_RTFL         0x2000          /* sc_flags: executing RTFL seq */
#endif  /* ibmrt */
#ifdef  i386
#define MAXSIGREGS      19
        int     sc_gs;
        int     sc_fs;
        int     sc_es;
        int     sc_ds;
        int     sc_edi;
        int     sc_esi;
        int     sc_ebp;
        int     sc_esp;
        int     sc_ebx;
        int     sc_edx;
        int     sc_ecx;
        int     sc_eax;
        int     sc_trapno;      /* err & trapno keep the context */
        int     sc_err;         /* switching code happy */
        int     sc_eip;
        int     sc_cs;
        int     sc_efl;
        int     sc_uesp;
        int     sc_ss;
#endif  /* i386 */
#ifdef  __mips__
        int     sc_pc;                  /* pc at time of signal */
        /*
         * General purpose registers
         */
        int     sc_regs[32];    /* processor regs 0 to 31 */
        int     sc_mdlo;        /* mul/div low */
        int     sc_mdhi;        /* mul/div high */
        /*
         * Floating point coprocessor state
         */
        int     sc_ownedfp;     /* fp has been used */
        int     sc_fpregs[32];  /* fp regs 0 to 31 */
        int     sc_fpc_csr;     /* floating point control and status reg */
        int     sc_fpc_eir;     /* floating point exception instruction reg */
        /*
         * END OF REGION THAT MUST AGREE WITH setjmp.h
         */
        /*
         * System coprocessor registers at time of signal
         */
        int     sc_cause;       /* cp0 cause register */
        int     sc_badvaddr;    /* cp0 bad virtual address */
        int     sc_badpaddr;    /* cpu bd bad physical address */
#define R_SP	29		/* also defined in arch/mips/machdep.c */
#define sc_sp	sc_regs[R_SP]
	size_t	sc_ssize;	/* stack size */
	caddr_t	sc_sbase;	/* stack start */
#endif  /* __mips__ */
#ifdef  __alpha
	long	sc_pc;			/* pc at time of signal */
	long	sc_ps;			/* psl to retore */
	long	sc_regs[32];		/* processor regs 0 to 31 */
	long	sc_ownedfp;		/* fp has been used */
	long	sc_fpregs[32];		/* fp regs 0 to 31 */
	unsigned long sc_fpcr;		/* floating point control register */
	unsigned long sc_fp_control;	/* software fpcr */
	/*
	 * END OF REGION THAT MUST AGREE WITH jmp_buf REGION IN setjmp.h
	 */
	long sc_reserved1;		/* reserved for user */
	long sc_reserved2;		/* reserved for user */
#define R_SP	30		/* also defined in arch/alpha/machdep.c */
#define sc_sp	sc_regs[R_SP]
	size_t	sc_ssize;	/* stack size */
	caddr_t	sc_sbase;	/* stack start */
	unsigned long sc_traparg_a0;	/* a0 argument to trap on exception */
	unsigned long sc_traparg_a1;	/* a1 argument to trap on exception */
	unsigned long sc_traparg_a2;	/* a2 argument to trap on exception */
	unsigned long sc_fp_trap_pc;	/* imprecise pc  */
	unsigned long sc_fp_trigger_sum; /* Exception summary at trigger pc */
	unsigned long sc_fp_trigger_inst; /* Instruction at trigger pc */
#endif  /* __alpha */
#ifdef sparc
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_npc;                 /* next pc to restore */
        int     sc_psr;                 /* psr to restore */
        int     sc_g1;                  /* register that must be restored */
        int     sc_o0;
#endif /* sparc */
};
#endif  /* __LANGUAGE_C__ */

#define SIGSTKSZ	(16384)
#define MINSIGSTKSZ	(4096)

/*
 * valid signal action values; other values => pointer to handler function 
 */
#define BADSIG		SIG_ERR
/*
 * valid flags define for sa_flag field of sigaction structure 
 */
#define	SA_ONSTACK	0x00000001	/* run on special signal stack */
#define SA_RESTART	0x00000002	/* restart system calls on sigs */
/* 
 * NOTE: The POSIX SA_NOCLDSTOP flag is defined above in this file 
 * as 0x00000004.
 */
#define SA_NODEFER	0x00000008	/* don't block while handling */
#define SA_RESETHAND	0x00000010	/* old sys5 style behavior */
#define SA_NOCLDWAIT	0x00000020	/* no zombies */
#define SA_SIGINFO	0x00000040	/* deliver siginfo to handler */

#define SIGCLD		SIGCHLD

/*
 * flag bits defined for parameter to psig and sendsig; bits indicate
 *  what context (where and how much) should be saved on signal delivery
 */       
#define	NO_VOLATILE_REGS	0x0001
#define	USE_SAVE_AREA		0x0002
/* 
 * macros to manipulate signal sets
 */
#define	SIGINITSET(set)		(set) = (sigset_t)0;

#define	SIGMASK(s)		sigmask(s)

#define	SIGEMPTYSET(set)	(set) = (sigset_t)0;

#define	SIGFILLSET(set)		(set) = (sigset_t)~0;

#define SIGDELSET(set, s)	(set) &= ~SIGMASK(s);

#define SIGADDSET(set, s)	(set) |= SIGMASK(s);

#define SIGISMEMBER(set,s)	(((set) & SIGMASK(s)) != (sigset_t)0)

#define SIGMASKSET(dest, mask)	((dest) &= ~(mask))

#define SIGORSET(dest, mask)	((dest) |= (mask))


/*
 * values in sv_onstack are interpreted identically to values in
 * sa_onstack for sigaction();  however, the following additional
 * names are defined for values in sv_onstack to be compatible with
 * old usage of sigvec() 
 */
#define NSIG	32			/* maximum number of signals */
#define SIG_STK		0x00000001	/* bit for using sigstack stack */
#define SIG_STD		0x00000002	/* bit for old style signals */
#define SV_ONSTACK	SA_ONSTACK  /* take signal on signal stack */
#define SV_INTERRUPT	SA_RESTART /* do not restart system on signal return */

#define _OLDSTYLE (2)
#define _ONSTACK  (1)
#define _teststyle(n)   ((n) & _OLDSTYLE) /** TRUE if Bell style signals. **/
#define _testonstack(n) ((n) & _ONSTACK)  /** TRUE if on user-sig stack.  **/
#define _setoldstyle(n) ((n) | _OLDSTYLE)
#define _setnewstyle(n) ((n) & ~_OLDSTYLE)
#define _setonstack(n)  ((n) | _ONSTACK)
#define _clronstack(n)  ((n) & ~_ONSTACK)

#define SIGMAX	NSIG

#if     defined(mc68000) || defined(__mc68000)
#define     ILL_ILLINSTR_FAULT  0x10    /* illegal instruction fault */
#define     ILL_PRIVVIO_FAULT   0x20    /* privilege violation fault */
#define     ILL_COPROCERR_FAULT 0x34    /* [coprocesser protocol error fault] */
#define     ILL_TRAP1_FAULT     0x84    /* trap #1 fault */
#define     ILL_TRAP2_FAULT     0x88    /* trap #2 fault */
#define     ILL_TRAP3_FAULT     0x8c    /* trap #3 fault */
#define     ILL_TRAP4_FAULT     0x90    /* trap #4 fault */
#define     ILL_TRAP5_FAULT     0x94    /* trap #5 fault */
#define     ILL_TRAP6_FAULT     0x98    /* trap #6 fault */
#define     ILL_TRAP7_FAULT     0x9c    /* trap #7 fault */
#define     ILL_TRAP8_FAULT     0xa0    /* trap #8 fault */
#define     ILL_TRAP9_FAULT     0xa4    /* trap #9 fault */
#define     ILL_TRAP10_FAULT    0xa8    /* trap #10 fault */
#define     ILL_TRAP11_FAULT    0xac    /* trap #11 fault */
#define     ILL_TRAP12_FAULT    0xb0    /* trap #12 fault */
#define     ILL_TRAP13_FAULT    0xb4    /* trap #13 fault */
#define     ILL_TRAP14_FAULT    0xb8    /* trap #14 fault */
#else   /* mc68000 */
#define     ILL_RESAD_FAULT     0x0     /* reserved addressing fault */
#define     ILL_PRIVIN_FAULT    0x1     /* privileged instruction fault */
#define     ILL_RESOP_FAULT     0x2     /* reserved operand fault */
#endif  /* mc68000 */
#ifdef  multimax
#define     ILL_FLAG_TRAP       0x3     /* flag trap taken */
#endif  /* multimax */
#ifdef  __mips__
#define     ILL_COPR_UNUSABLE   0x3     /* coprocessor unusable */
#endif  /* __mips__ */
#ifdef  __alpha
#define     ILL_INST_FAULT      0x3     /* Illegal instruction */
#define     ILL_MODE_FAULT      0x4     /* Illegal mode - VMSPAL only */
#endif  /* __alpha */

#if     defined(mc68000) || defined(__mc68000)
#define     EMT_EMU1010         0x28    /* line 1010 emulator trap */
#define     EMT_EMU1111         0x2c    /* line 1111 emulator trap */
#define     FPE_INTDIV_TRAP     0x14    /* integer divide by zero */
#define     FPE_CHKINST_TRAP    0x18    /* CHK [CHK2] instruction */
#define     FPE_TRAPV_TRAP      0x1c    /* TRAPV [cpTRAPcc TRAPcc] instr */
#define     FPE_FLTBSUN_TRAP    0xc0    /* [branch or set on unordered cond] */
#define     FPE_FLTINEX_TRAP    0xc4    /* [floating inexact result] */
#define     FPE_FLTDIV_TRAP     0xc8    /* [floating divide by zero] */
#define     FPE_FLTUND_TRAP     0xcc    /* [floating underflow] */
#define     FPE_FLTOPERR_TRAP   0xd0    /* [floating operand error] */
#define     FPE_FLTOVF_TRAP     0xd4    /* [floating overflow] */
#define     FPE_FLTNAN_TRAP     0xd8    /* [floating Not-A-Number] */
#else   /* mc68000 */
#ifdef  sparc
#define     FPE_INTDIV_TRAP     0x14    /* integer divide by zero */
#define     FPE_FLTINEX_TRAP    0xc4    /* [floating inexact result] */
#define     FPE_FLTDIV_TRAP     0xc8    /* [floating divide by zero] */
#define     FPE_FLTUND_TRAP     0xcc    /* [floating underflow] */
#define     FPE_FLTOPERR_TRAP   0xd0    /* [floating operand error] */
#define     FPE_FLTOVF_TRAP     0xd4    /* [floating overflow] */
#else   /* sparc */
#define     FPE_INTOVF_TRAP     0x1     /* integer overflow */
#define     FPE_INTDIV_TRAP     0x2     /* integer divide by zero */
#define     FPE_FLTOVF_TRAP     0x3     /* floating overflow */
#define     FPE_FLTDIV_TRAP     0x4     /* floating/decimal divide by zero */
#define     FPE_FLTUND_TRAP     0x5     /* floating underflow */
#define     FPE_DECOVF_TRAP     0x6     /* decimal overflow */
#define     FPE_SUBRNG_TRAP     0x7     /* subscript out of range */
#define     FPE_FLTOVF_FAULT    0x8     /* floating overflow fault */
#define     FPE_FLTDIV_FAULT    0x9     /* divide by zero floating fault */
#define     FPE_FLTUND_FAULT    0xa     /* floating underflow fault */
#endif  /* sparc */
#endif  /* mc68000 */
#if     sun
#define     FPE_FPA_ENABLE      0x400   /* [FPA not enabled] */
#define     FPE_FPA_ERROR       0x404   /* [FPA arithmetic exception] */
#endif  /* sun */
#ifdef  multimax
#define     FPE_ILLINST_FAULT   0xb     /* Illegal FPU instruction */
#define     FPE_INVLOP_FAULT    0xc     /* Invalid operation */
#define     FPE_INEXACT_FAULT   0xd     /* Inexact result */
#define     FPE_OPERAND_FAULT   0xe     /* Operand fault        */
#endif  /* multimax */
#ifdef  sparc
#define     BUS_ALIGN           0x1     /* alignment error */
#define     EMT_TAG             0x0a    /* tag overflow */
#define     ILL_STACK           0x00    /* bad stack */
#endif  /* sparc */
#if defined(__mips__) || defined(__alpha)
#define     FPE_UNIMP_FAULT     0xb     /* Unimplemented FPU instruction */
#define     FPE_INVALID_FAULT   0xc     /* Invalid operation */
#define     FPE_INEXACT_FAULT   0xd     /* Inexact result */
#endif  /* __mips__ || __alpha */

#if defined(__alpha)
#define	    FPE_HPARITH_TRAP	0xe	/* High performance trap */
#define     FPE_INTOVF_FAULT    0xf     /* Integer Overflow fault */
#define	    FPE_ILLEGAL_SHADOW_TRAP 0x10 /* Illegal Trap Shadow Trap */
#define	    FPE_GENTRAP		0x11	/* */
#endif /* __alpha */


/*
 * Special signal number for intermediate signal handler.
 */
#ifdef  multimax
#define SIGCATCHALL 0x400
#endif  /* multimax */

/*
 * Codes for the mips/alpha break instruction.
 * (probably belong in a machine-specific header file)
 */
#if defined (mips) || defined (__alpha)
#define BRK_USERBP        0       /* user bp (used by debuggers) */
#define BRK_KERNELBP      1       /* kernel bp (used by prom) */
#define BRK_ABORT         2       /* no longer used */
#define BRK_BD_TAKEN      3       /* for taken bd emulation */
#define BRK_BD_NOTTAKEN   4       /* for not taken bd emulation */
#define BRK_SSTEPBP       5       /* user bp (used by debuggers) */
#define BRK_OVERFLOW      6       /* overflow check */
#define BRK_DIVZERO       7       /* divide by zero check */
#define BRK_RANGE         8       /* range error check */
#define BRK_STACKOVERFLOW 9	  /* used by ada */
#endif	/* mips or __alpha */

#if !defined(_KERNEL) && defined(__LANGUAGE_C__)
#ifdef  _NO_PROTO 
extern int sigblock();
extern int sigpause();
extern int sigreturn();
extern int sigsetmask();
extern int sigstack();
extern int siginterrupt();
/* SVR3, SVR4 */
extern int sigaltstack();
extern void (*sigset())();
extern int sighold();
extern int sigrelse();
extern int sigignore();
extern int sigsendset();
extern int sigsend();
#else
#if defined(__cplusplus)
extern "C" {
#endif

extern int sigblock(int);
extern int sigpause(int);
extern int sigreturn(struct sigcontext *);
extern int sigsetmask(int);
extern int sigstack(struct sigstack *, struct sigstack *);
extern int siginterrupt(int, int);

/* SVR3, SVR4 */
extern void (*sigset(int, void (*)(int)))(int);
extern int sighold(int);
extern int sigrelse(int);
extern int sigignore(int);
extern int sigaltstack(stack_t *, stack_t *);
#include <sys/procset.h>
extern int sigsendset(const procset_t *, int);
extern int sigsend(idtype_t, id_t, int);

#if defined(__cplusplus)
}
#endif
#endif  /* _NO_PROTO */
#endif  /* !_KERNEL && __LANGUAGE_C__ */

#endif /* _OSF_SOURCE */

#endif /* _SYS_SIGNAL_H_ */
