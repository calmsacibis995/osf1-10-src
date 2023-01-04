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
static char	*sccsid = "@(#)$RCSfile: sysmips.c,v $ $Revision: 1.2.3.4 $ (DEC) $Date: 1992/04/14 13:39:39 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from sysmips.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 * MIPS specific syscall interface.
 *
 * To Use:
 *
 * Get Yourself a Number from the list, if you want to write any
 * code which will reside in sysmips(), use a number from 0x0 to 0xff.
 * If you just need a function vector, use a number above 0x100.
 *
 * You will be called using the same convention as the normal system
 * calls -- with three arguments:
 *	call(p, args, retval)
 *		struct proc *p;
 *		void *args;		the arguments 
 *		int *retval;		return values (used to be u.u_rval*) 
 * The return value should be 0 for success, a valid errno for errors.
 *
 * Have a homebrew :-)
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/proc.h>
#include <machine/utsname.h>
#include <machine/hwconf.h>

/*
 * Defines for MIPS specific system calls.
 */

#define MIPS_VECTOR_SIZE 0x200
#define MIPS_VECTOR_DIVIDER 0x100

/* those that are implemented all or in part in the sysmips() routine */
#define MIPS_UNAME	0x000
#define MIPS_FPSIGINTR	0x001
#define MIPS_FPU	0x002
#define MIPS_FIXADE	0x003

/* those that are entirely implemented in a broken out procedure */
#define MIPS_KOPT	0x100
#define MIPS_HWCONF	0x101
#define MIPS_GETRUSAGE	0x102
#define MIPS_WAIT3	0x103
#define MIPS_CACHEFLUSH	0x104
#define MIPS_CACHECTL	0x105		/* defunct */


int errsys();
int mipskopt();
int mipshwconf();
int mips_getrusage();
int mips_wait();
int cacheflush();
int fptype_word;
extern /* thread_t */ fpowner_array[];

int (*func_vector[])() = {
	mipskopt,
	mipshwconf,
	mips_getrusage,
	mips_wait,
	cacheflush,
	errsys,
	};

struct sm_utsname sm_utsname = {
	"amnesia",
	"amnesia",
	R_2_0,
	V_MACH,
	M_MIPS,
#if defined(PMAX)
	MT_DEC3100,
#else
#if defined(R2300)
	MT_M500,
#endif
#if defined(R2600)
	MT_M800,
#endif
#endif /* PMAX */
	BR_43_BSD
	};

sysmips(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	register struct args {
		int vector;
		int arg1,arg2,arg3,arg4;
	} *uap = args;
	register int vector;
	extern char		hostname[];
	extern short		hostnamelen;
	extern char		domainname[];
	extern short		domainnamelen;
	int			error;

	vector = uap->vector;

	/* Warn about unsupported syscalls */
	switch (vector) {
	case MIPS_GETRUSAGE:
	case MIPS_WAIT3:
	case MIPS_CACHEFLUSH:
		break;
	default:
		printf("Warning: process %d (%s) invoked sysmips(%x %x %x %x %x)\n",
			p->p_pid, u.u_comm,
			vector, uap->arg1, uap->arg2, uap->arg3, uap->arg4);
	}

	if (vector < 0 || vector > MIPS_VECTOR_SIZE)
		return (EINVAL);

	if (vector >= MIPS_VECTOR_DIVIDER)	{
		if ((vector - MIPS_VECTOR_DIVIDER) >
		    (sizeof(func_vector) / sizeof(func_vector[0])))
			return (EINVAL);

		return ((*func_vector[vector - MIPS_VECTOR_DIVIDER])
			(p, &uap->arg1, retval));
	} else {
		switch (vector) {

		case MIPS_UNAME:
/*  mipsco system specific code commented out.
			switch (hwconf.cpubd_type) {
			case BRDTYPE_R2300:
				strcpy(sm_utsname.m_type, "m500");
				break;
			case BRDTYPE_R2600:
				strcpy(sm_utsname.m_type, "m800");
				break;
			case BRDTYPE_R2800:
				strcpy(sm_utsname.m_type, "m1000");
				break;
			}
****/
			strncpy(sm_utsname.sysname, hostname, SYS_NMLN);
			strncpy(sm_utsname.nodename, hostname, SYS_NMLN);
			return (copyout(&sm_utsname, uap->arg1,
				sizeof(sm_utsname)));

		case MIPS_FPSIGINTR:
#ifdef	notyet	/* Not needed */
			p->p_fp = uap->arg1;
#endif
			break;

		case MIPS_FPU:
#if	SEC_BASE
			if (!privileged(SEC_SYSATTR, EPERM))
				return (EPERM);
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				return (error);
#endif

			if (uap->arg1 != 0)
				fptype_word = hwconf.fpu_processor.ri_uint & 0xff00;
			else {
				if (fpowner_array[cpu_number()] != 0)
					checkfp(fpowner_array[cpu_number()], 0);
				fptype_word = 0;
			}
			break;

		case MIPS_FIXADE:
#ifdef	notdef	/* Standard */
			if (uap->arg1 != 0)
				p->p_flag |= SFIXADE;
			else
				p->p_flag &= ~SFIXADE;
#endif
			break;

		default:
			return (EINVAL);
		}
	}
	return (0);
}


/*
 * mips_getrusage() provides the same functionality as getrusage().
 * The idea was to be able to return more info someday, but it never happened.
 */
mips_getrusage(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		int who;
		struct rusage *rusage;
		int rusage_size;
	} *uap = (struct args *) args;

	if (uap->rusage_size < 0 || uap->rusage_size > sizeof(struct rusage)){
		return (EINVAL);
	}
	return (getrusage(p, args, retval));
}

#include <sys/wait.h>

/*
 * mips_wait() provides the same functionality as wait() above but allows
 * the rusage structure to grow so that mips specific stats can be returned
 * in the future.
 */
mips_wait(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	register struct args {
		int	*status;
		int	options;
		struct	rusage_dev *rusage;
		int	rusage_size;
	} *uap = (struct args *) args;

	struct args1 {
		int	pid;
		int	*status;
		int	options;
		struct	rusage_dev *rusage;
		int	compat;
	} uap1;

	if(uap->rusage_size < 0 || uap->rusage_size > sizeof(struct rusage)){
		return (EINVAL);
	}

	uap1.pid = WAIT_ANY;
	uap1.status = uap->status;
	uap1.options = uap->options;
	uap1.rusage = uap->rusage;
	uap1.compat = 1;
	return (waitf(p, &uap1, retval));
}
