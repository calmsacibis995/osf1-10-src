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
static char	*sccsid = "@(#)$RCSfile: crt0.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:46:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)

#endif LIBC_SCCS and not lint

/*
 *	C start up routine.
 *	Robert Henry, UCB, 20 Oct 81
 *
 *	We make the following (true) assumptions:
 *	1) when the kernel calls start, it does a jump to location 2,
 *	and thus avoids the register save mask.  We are NOT called
 *	with a calls!  see sys1.c:setregs().
 *	2) The only register variable that we can trust is sp,
 *	which points to the base of the kernel calling frame.
 *	Do NOT believe the documentation in exec(2) regarding the
 *	values of fp and ap.
 *	3) We can allocate as many register variables as we want,
 *	and don't have to save them for anybody.
 *	4) Because of the ways that asm's work, we can't have
 *	any automatic variables allocated on the stack, because
 *	we must catch the value of sp before any automatics are
 *	allocated.
 */

#include <machine/asm.h>

char **environ = (char **)0;

int errno;

#ifdef paranoid
static int fd;
#endif paranoid

#if	MACH
int	(*mach_init_routine)();
int	(*_cthread_init_routine)();
int	(*_cthread_exit_routine)();
int	(*_StrongBox_init_routine)();
#endif	MACH


extern	unsigned char	etext;
extern	unsigned char	eprol;
__start()
{
	struct kframe {
		int	kargc;
		char	*kargv[1];	/* size depends on kargc */
		char	kargstr[1];	/* size varies */
		char	kenvstr[1];	/* size varies */
	};
	/*
	 *	ALL REGISTER VARIABLES!!!
	 */
	register int r11;		/* needed for init */
	register struct kframe *kfp;	/* r10 */
	register char **targv;
	register char **argv;
	int argc;

#ifdef lint
	kfp = 0;
	initcode = initcode = 0;
#else not lint
#ifdef	__wheeze__
	asm("	leal	4(%ebp),%edi");	/* catch it quick */
#else	__wheeze__
#define Entry_sp() \
({ int _spl__, _tmp1__; \
	asm ("leal 4(%%ebp), %0" : "=r" (_spl__) : "r" (_tmp1__)); \
	_spl__; })

	kfp = (struct kframe *)Entry_sp();
#endif	__wheeze__
#endif not lint
	for (argv = targv = &kfp->kargv[0]; *targv++; /* void */)
		/* void */ ;
	if (targv >= (char **)(*argv))
		--targv;
	environ = targv;
#if	MACH
	if (mach_init_routine)
		(void) mach_init_routine();
#endif	MACH
asm("eprol:");

#ifdef paranoid
	/*
	 * The standard I/O library assumes that file descriptors 0, 1, and 2
	 * are open. If one of these descriptors is closed prior to the start 
	 * of the process, I/O gets very confused. To avoid this problem, we
	 * insure that the first three file descriptors are open before calling
	 * main(). Normally this is undefined, as it adds two unnecessary
	 * system calls.
	 */
	do	{
		fd = open("/dev/null", 2);
	} while (fd >= 0 && fd < 3);
	close(fd);
#endif paranoid


#ifdef MCRT0
	monstartup(&eprol, &etext);
#endif MCRT0
	errno = 0;
#if	MACH
	if (_cthread_init_routine) (*_cthread_init_routine)();
	if (_StrongBox_init_routine) (*_StrongBox_init_routine)();
#endif

         argc = main(kfp->kargc, argv, environ);
        
#if    MACH
	if(_cthread_exit_routine)
             _cthread_exit_routine(argc);
#endif	
	exit(argc);

}

#ifdef MCRT0
exit(code)
	register int code;	/* r11 */
{
	monitor(0);
	_cleanup();
	_exit(code);
}
#endif


#ifdef CRT0
/*
 * null mcount and moncontrol,
 * just in case some routine is compiled for profiling
 */
moncontrol(val)
	int val;
{

}
asm("	.globl	_mcount");
asm("_mcount:	ret");
#endif CRT0
