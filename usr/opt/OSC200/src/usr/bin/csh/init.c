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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: init.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/10 16:50:45 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.6  com/cmd/csh/init.c, bos320 5/10/91 15:38:32";
 */ 

#include "local.h"
#include "csh_msg.h"
#include "sh.h"

/*
 * Note that for the hashing algorithm to work
 * properly the following list must be in alpha
 * order.
 * 
 * This casting is used here to initialize the bfunc
 * array that follows. In no way is it intended
 * to apply to casting that already exists in the sh.h. 
 */

struct biltins bfunc[] =  {
	(uchar_t *)"@",		dolet,		0,	INF,
	(uchar_t *)"alias",	doalias,	0,	INF,
	(uchar_t *)"bg",	dobg,		0,	INF,
#ifdef CMDEDIT
        (uchar_t *)"bind-to-key",  dobind,         0,      INF,
#endif
	(uchar_t *)"break",	dobreak,	0,	0,
	(uchar_t *)"breaksw",	doswbrk,	0,	0,
	(uchar_t *)"case",	dozip,		0,	1,
	(uchar_t *)"cd",	dochngd,	0,	1,
	(uchar_t *)"chdir",	dochngd,	0,	1,
	(uchar_t *)"continue",	docontin,	0,	0,
	(uchar_t *)"default",	dozip,		0,	0,
	(uchar_t *)"dirs",	dodirs,		0,	1,
	(uchar_t *)"echo",	doecho,		0,	INF,
	(uchar_t *)"else",	doelse,		0,	INF,
	(uchar_t *)"end",	doend,		0,	0,
	(uchar_t *)"endif",	dozip,		0,	0,
	(uchar_t *)"endsw",	dozip,		0,	0,
	(uchar_t *)"eval",	doeval,		0,	INF,
	(uchar_t *)"exec",	execash,	1,	INF,
	(uchar_t *)"exit",	doexit,		0,	INF,
	(uchar_t *)"fg",	dofg,		0,	INF,
	(uchar_t *)"foreach",	doforeach,	3,	INF,
	(uchar_t *)"glob",	doglob,		0,	INF,
	(uchar_t *)"goto",	dogoto,		1,	1,
	(uchar_t *)"hashstat",	hashstat,	0,	0,
	(uchar_t *)"history",	dohist,		0,	2,
	(uchar_t *)"if",	doif,		1,	INF,
/* Merge in DEC OSF 1.2 bug fix by Dave Gray
*	(uchar_t *)"inlib",	doinlib,	1,	1,
*/
	(uchar_t *)"jobs",	dojobs,		0,	1,
	(uchar_t *)"kill",	dokill,		1,	INF,
	(uchar_t *)"limit",	dolimit,	0,	3,
	(uchar_t *)"login",	dologin,	0,	INF,
	(uchar_t *)"logout",	dologout,	0,	0,
	(uchar_t *)"newgrp",	donewgrp,	0,	1,
	(uchar_t *)"nice",	donice,		0,	INF,
	(uchar_t *)"nohup",	donohup,	0,	INF,
	(uchar_t *)"notify",	donotify,	0,	INF,
	(uchar_t *)"onintr",	doonintr,	0,	2,
	(uchar_t *)"popd",	dopopd,		0,	1,
	(uchar_t *)"pushd",	dopushd,	0,	1,
	(uchar_t *)"rehash",	dohash,		0,	0,
/* Merge in DEC OSF 1.2 bug fix by Dave Gray
*	(uchar_t *)"rmlib",	dormlib,	1,	1,
*/

	(uchar_t *)"repeat",	dorepeat,	2,	INF,
	(uchar_t *)"set",	doset,		0,	INF,
	(uchar_t *)"setenv",	dosetenv,	0,	2,
	(uchar_t *)"shift",	shift,		0,	1,
	(uchar_t *)"source",	dosource,	1,	2,
	(uchar_t *)"stop",	dostop,		0,	INF,
	(uchar_t *)"suspend",	dosuspend,	0,	0,
	(uchar_t *)"switch",	doswitch,	1,	INF,
	(uchar_t *)"time",	dotime,		0,	INF,
	(uchar_t *)"umask",	doumask,	0,	1,
	(uchar_t *)"unalias",	unalias,	1,	INF,
	(uchar_t *)"unhash",	dounhash,	0,	0,
	(uchar_t *)"unlimit",	dounlimit,	0,	INF,
	(uchar_t *)"unset",	unset,		1,	INF,
	(uchar_t *)"unsetenv",	dounsetenv,	1,	INF,
	(uchar_t *)"wait",	dowait,		0,	0,
	(uchar_t *)"which",	dowhich,	1,	INF,	/* 001 RNF  */
	(uchar_t *)"while",	dowhile,	1,	INF,
	(uchar_t *)0,		0,		0,	0,
};


struct srch srchn [] = {
	(uchar_t *)"@",		ZLET,
	(uchar_t *)"break",	ZBREAK,
	(uchar_t *)"breaksw",	ZBRKSW,
	(uchar_t *)"case",	ZCASE,
	(uchar_t *)"default", 	ZDEFAULT,
	(uchar_t *)"else",	ZELSE,
	(uchar_t *)"end",	ZEND,
	(uchar_t *)"endif",	ZENDIF,
	(uchar_t *)"endsw",	ZENDSW,
	(uchar_t *)"exit",	ZEXIT,
	(uchar_t *)"foreach", 	ZFOREACH,
	(uchar_t *)"goto",	ZGOTO,
	(uchar_t *)"if",	ZIF,
	(uchar_t *)"label",	ZLABEL,
	(uchar_t *)"set",	ZSET,
	(uchar_t *)"switch",	ZSWITCH,
	(uchar_t *)"while",	ZWHILE,
	(uchar_t *)0,		0,
};

struct mesg mesg[] = {
        (uchar_t *)0,	     0,		   (uchar_t *)0,
        (uchar_t *)"HUP",    M_SIGHUP,	   (uchar_t *)"Hangup",
        (uchar_t *)"INT",    M_SIGINT,	   (uchar_t *)"Interrupt",
        (uchar_t *)"QUIT",   M_SIGQUIT,	   (uchar_t *)"Quit",
        (uchar_t *)"ILL",    M_SIGILL,	   (uchar_t *)"Illegal instruction",
        (uchar_t *)"TRAP",   M_SIGTRAP,	   (uchar_t *)"Trace/BPT trap",
        (uchar_t *)"ABRT",   M_SIGABRT,	   (uchar_t *)"Abort process",
        (uchar_t *)"EMT",    M_SIGEMT,	   (uchar_t *)"EMT trap",
        (uchar_t *)"FPE",    M_SIGFPE,	   (uchar_t *)"Floating exception",
        (uchar_t *)"KILL",   M_SIGKILL,	   (uchar_t *)"Killed",
        (uchar_t *)"BUS",    M_SIGBUS,	   (uchar_t *)"Bus error",
        (uchar_t *)"SEGV",   M_SIGSEGV,	   (uchar_t *)"Segmentation fault",
        (uchar_t *)"SYS",    M_SIGSYS,	   (uchar_t *)"Bad system call",
        (uchar_t *)"PIPE",   M_SIGPIPE,	   (uchar_t *)"Broken pipe",
        (uchar_t *)"ALRM",   M_SIGALRM,	   (uchar_t *)"Alarm clock",
        (uchar_t *)"TERM",   M_SIGTERM,	   (uchar_t *)"Terminated",
        (uchar_t *)"URG",    M_SIGURG,	   (uchar_t *)"Urgent I/O condition",
        (uchar_t *)"STOP",   M_SIGSTOP,	   (uchar_t *)"Suspended (signal)",
        (uchar_t *)"TSTP",   M_SIGTSTP,	   (uchar_t *)"Suspended",
        (uchar_t *)"CONT",   M_SIGCONT,	   (uchar_t *)"Continued",
        (uchar_t *)"CHLD",   M_SIGCHLD,	   (uchar_t *)"Child exited",
        (uchar_t *)"TTIN",   M_SIGTTIN,	   (uchar_t *)"Suspended (tty input)",
        (uchar_t *)"TTOU",   M_SIGTTOU,	   (uchar_t *)"Suspended (tty output)",
        (uchar_t *)"IO",     M_SIGIO,	   (uchar_t *)"I/O possible",
        (uchar_t *)"XCPU",   M_SIGXCPU,	   (uchar_t *)"Cputime limit exceeded",
        (uchar_t *)"XFSZ",   M_SIGXFSZ,	   (uchar_t *)"Filesize limit exceeded",
        (uchar_t *)"VTALRM", M_SIGVTALARM, (uchar_t *)"Virtual timer expired",
        (uchar_t *)"PROF",   M_SIGROF,	   (uchar_t *)"Profiling timer expired",
        (uchar_t *)"WINCH",  M_SIGWINCH,   (uchar_t *)"Window size changed",
        (uchar_t *)"INFO",   M_SIGINFO,	   (uchar_t *)"Information request",
        (uchar_t *)"USR1",   M_SIGUSR1,    (uchar_t *)"User defined signal 1",
        (uchar_t *)"USR2",   M_SIGUSR2,	(uchar_t *)"User defined signal 2"
};
