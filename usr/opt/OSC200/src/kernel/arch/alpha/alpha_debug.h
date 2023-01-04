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

/*	"@(#)alpha_debug.h	9.1	(ULTRIX/OSF)	10/21/91" *\

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Debug macros.
 */

#ifdef __LANGUAGE_C__

#ifdef notneeded
struct xprbuf {
	char *xp_msg;
	unsigned xp_arg1, xp_arg2, xp_arg3, xp_arg4;
	unsigned xp_timestamp;
	unsigned xp_pid, xp_tlbpid;
};
#endif /* notneeded */

#ifdef KERNEL

#ifdef ASSERTIONS
#define ASSERT(EX) { if (EX) ; else assfail("EX", __FILE__, __LINE__) }
#else /* ASSERTIONS */
#define ASSERT(EX)
#endif /* ASSERTIONS */

#define	XPRINTF(flags, format, arg1, arg2, arg3, arg4)
#endif /* KERNEL */

#endif /* __LANGUAGE_C__ */

/*
 * flags
 */
#ifdef notneeded
#define XPR_CLOCK	0x00000001	/* Clock interrupt handler */
#define XPR_TLB		0x00000002	/* TLB miss handler */
#define XPR_INIT	0x00000004	/* routines called during init */
#define XPR_SCHED	0x00000008	/* Scheduler */
#define XPR_PROCESS	0x00000010	/* newproc/fork */
#define XPR_EXEC	0x00000020	/* Exec */
#define XPR_SYSCALL	0x00000040	/* System calls */
#define XPR_TRAP	0x00000080	/* Trap handler */
#define XPR_NOFAULT	0x00000100	/* Nofault bus error */
#define XPR_VM		0x00000200	/* VM */
#define XPR_SWAP	0x00000400	/* swapin/swapout */
#define XPR_SWTCH	0x00000800	/* swtch, setrq, remrq */
#define	XPR_DISK	0x00001000	/* disk i/o */
#define	XPR_TTY		0x00002000	/* mux i/o */
#define	XPR_TAPE	0x00004000	/* tape i/o */
#define	XPR_BIO		0x00008000	/* blk i/o */
#define	XPR_INTR	0x00010000	/* interrupt handling */
#define	XPR_RMAP	0x00020000	/* resource map handling */
#define	XPR_TEXT	0x00040000	/* shared text stuff */
#define	XPR_CACHE	0x00080000	/* cache handling */
#define	XPR_NFS		0x00100000	/* nfs */
#define	XPR_RPC		0x00200000	/* rpc */
#define	XPR_SIGNAL	0x00400000	/* signal handling */
#define	XPR_FPINTR	0x00800000	/* fp interrupt handling */
#define XPR_SM          0x01000000      /* Shared memory */
/*
 * options for mipskopt system call
 */
#define	KOPT_GET	1		/* get kernel option */
#define	KOPT_SET	2		/* set kernel option */
#define	KOPT_BIS	3		/* or in new option value */
#define	KOPT_BIC	4		/* clear indicated bits */
#endif /* notneeded */

#ifdef __LANGUAGE_C__

#ifdef KERNEL
#ifdef notneeded
extern struct reg_values pstat_values[];
extern struct reg_values sig_values[];
extern struct reg_values imask_values[];
extern struct reg_values exc_values[];
extern struct reg_values fileno_values[];
extern struct reg_values prot_values[];
extern struct reg_values syscall_values[];
extern struct reg_desc sr_desc[];
extern struct reg_desc exccode_desc[];
extern struct reg_desc cause_desc[];
extern struct reg_desc tlbhi_desc[];
extern struct reg_desc tlblo_desc[];
extern struct reg_desc tlbinx_desc[];
extern struct reg_desc tlbrand_desc[];
extern struct reg_desc tlbctxt_desc[];
extern struct reg_desc pte_desc[];
#endif /* notneeded */
#endif /* KERNEL */
#endif /* __LANGUAGE_C__ */
