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
 *	@(#)$RCSfile: externs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:57:33 $
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
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#ifndef	BSD
# define BSD 44
#endif


#define _USE_NEW_TTY

#include <stdio.h>
#include <setjmp.h>
#include <sys/ioctl.h>

#include <sys/termios.h>

#define	SUBBUFSIZE	256

extern int errno;		/* outside this world */

extern char
    *strcat(),
    *strcpy();			/* outside this world */

extern int
    flushout,		/* flush output */
    connected,		/* Are we connected to the other side? */
    globalmode,		/* Mode tty should be in */
    In3270,			/* Are we in 3270 mode? */
    telnetport,		/* Are we connected to the telnet port? */
    localflow,		/* Flow control handled locally */
    localchars,		/* we recognize interrupt/quit */
    donelclchars,		/* the user has set "localchars" */
    showoptions,
    net,		/* Network file descriptor */
    tin,		/* Terminal input file descriptor */
    tout,		/* Terminal output file descriptor */
    crlf,		/* Should '\r' be mapped to <CR><LF> (or <CR><NUL>)? */
    autoflush,		/* flush output when interrupting? */
    autosynch,		/* send interrupt characters with SYNCH? */
    SYNCHing,		/* Is the stream in telnet SYNCH mode? */
    donebinarytoggle,	/* the user has put us in binary */
    dontlecho,		/* do we suppress local echoing right now? */
    crmod,
    netdata,		/* Print out network data flow */
    prettydump,		/* Print "netdata" output in user readable format */
#if	defined(unix)
#if	defined(TN3270)
    cursesdata,		/* Print out curses data flow */
    apitrace,		/* Trace API transactions */
#endif	/* defined(TN3270) */
    termdata,		/* Print out terminal data flow */
#endif	/* defined(unix) */
    debug;			/* Debug level */

extern cc_t escape;     /* Escape to command mode */
#ifdef  KLUDGELINEMODE
extern cc_t echoc;      /* Toggle local echoing */
#endif

extern char
    *prompt;		/* Prompt for command. */

extern char
    doopt[],
    dont[],
    will[],
    wont[],
    options[],		/* All the little options */
    *hostname;		/* Who are we connected to? */

/*
 * We keep track of each side of the option negotiation.
 */

#define	MY_STATE_WILL		0x01
#define	MY_WANT_STATE_WILL	0x02
#define	MY_STATE_DO		0x04
#define	MY_WANT_STATE_DO	0x08

/*
 * Macros to check the current state of things
 */

#define	my_state_is_do(opt)		(options[opt]&MY_STATE_DO)
#define	my_state_is_will(opt)		(options[opt]&MY_STATE_WILL)
#define my_want_state_is_do(opt)	(options[opt]&MY_WANT_STATE_DO)
#define my_want_state_is_will(opt)	(options[opt]&MY_WANT_STATE_WILL)

#define	my_state_is_dont(opt)		(!my_state_is_do(opt))
#define	my_state_is_wont(opt)		(!my_state_is_will(opt))
#define my_want_state_is_dont(opt)	(!my_want_state_is_do(opt))
#define my_want_state_is_wont(opt)	(!my_want_state_is_will(opt))

#define	set_my_state_do(opt)		{options[opt] |= MY_STATE_DO;}
#define	set_my_state_will(opt)		{options[opt] |= MY_STATE_WILL;}
#define	set_my_want_state_do(opt)	{options[opt] |= MY_WANT_STATE_DO;}
#define	set_my_want_state_will(opt)	{options[opt] |= MY_WANT_STATE_WILL;}

#define	set_my_state_dont(opt)		{options[opt] &= ~MY_STATE_DO;}
#define	set_my_state_wont(opt)		{options[opt] &= ~MY_STATE_WILL;}
#define	set_my_want_state_dont(opt)	{options[opt] &= ~MY_WANT_STATE_DO;}
#define	set_my_want_state_wont(opt)	{options[opt] &= ~MY_WANT_STATE_WILL;}

/*
 * Make everything symetrical
 */

#define	HIS_STATE_WILL			MY_STATE_DO
#define	HIS_WANT_STATE_WILL		MY_WANT_STATE_DO
#define HIS_STATE_DO			MY_STATE_WILL
#define HIS_WANT_STATE_DO		MY_WANT_STATE_WILL

#define	his_state_is_do			my_state_is_will
#define	his_state_is_will		my_state_is_do
#define his_want_state_is_do		my_want_state_is_will
#define his_want_state_is_will		my_want_state_is_do

#define	his_state_is_dont		my_state_is_wont
#define	his_state_is_wont		my_state_is_dont
#define his_want_state_is_dont		my_want_state_is_wont
#define his_want_state_is_wont		my_want_state_is_dont

#define	set_his_state_do		set_my_state_will
#define	set_his_state_will		set_my_state_do
#define	set_his_want_state_do		set_my_want_state_will
#define	set_his_want_state_will		set_my_want_state_do

#define	set_his_state_dont		set_my_state_wont
#define	set_his_state_wont		set_my_state_dont
#define	set_his_want_state_dont		set_my_want_state_wont
#define	set_his_want_state_wont		set_my_want_state_dont


extern FILE
    *NetTrace;		/* Where debugging output goes */
extern unsigned char
    NetTraceFile[];	/* Name of file where debugging output goes */
extern void
    SetNetTrace();	/* Function to change where debugging goes */

extern jmp_buf
    peerdied,
    toplevel;		/* For error conditions. */

extern void
    command(),
    dosynch(),
    get_status(),
    Dump(),
    init_3270(),
    printoption(),
    printsub(),
    sendnaws(),
    setconnmode(),
    setcommandmode(),
    setneturg(),
    sys_telnet_init(),
    telnet(),
    TerminalFlushOutput(),
    TerminalNewMode(),
    TerminalRestoreState(),
    TerminalSaveState(),
    tninit(),
    upcase(),
    willoption(),
    wontoption();


extern struct	termios new_tc;

# define termEofChar		new_tc.c_cc[VEOF]
# define termEraseChar		new_tc.c_cc[VERASE]
# define termIntChar		new_tc.c_cc[VINTR]
# define termKillChar		new_tc.c_cc[VKILL]
# define termQuitChar		new_tc.c_cc[VQUIT]

# ifndef	VSUSP
extern cc_t termSuspChar;
# else
#  define termSuspChar		new_tc.c_cc[VSUSP]
# endif
# if    !defined(VFLUSHO) && defined(VDISCARD)
#  define VFLUSHO VDISCARD
# endif
# ifndef	VFLUSHO
extern cc_t termFlushChar;
# else
#  define termFlushChar		new_tc.c_cc[VFLUSHO]
# endif
# ifndef VWERASE
extern cc_t termWerasChar;
# else
#  define termWerasChar		new_tc.c_cc[VWERASE]
# endif
# ifndef	VREPRINT
extern cc_t termRprntChar;
# else
#  define termRprntChar		new_tc.c_cc[VREPRINT]
# endif
# ifndef	VLNEXT
extern cc_t termLiteralNextChar;
# else
#  define termLiteralNextChar	new_tc.c_cc[VLNEXT]
# endif
# ifndef	VSTART
extern cc_t termStartChar;
# else
#  define termStartChar		new_tc.c_cc[VSTART]
# endif
# ifndef	VSTOP
extern cc_t termStopChar;
# else
#  define termStopChar		new_tc.c_cc[VSTOP]
# endif
# ifndef	VEOL
extern cc_t termForw1Char;
# else
#  define termForw1Char		new_tc.c_cc[VEOL]
# endif
# ifndef	VEOL2
extern cc_t termForw2Char;
# else
#  define termForw2Char		new_tc.c_cc[VEOL]
# endif

#  define termEofCharp		&termEofChar
#  define termEraseCharp	&termEraseChar
#  define termIntCharp		&termIntChar
#  define termKillCharp		&termKillChar
#  define termQuitCharp		&termQuitChar
#  define termSuspCharp		&termSuspChar
#  define termFlushCharp	&termFlushChar
#  define termWerasCharp	&termWerasChar
#  define termRprntCharp	&termRprntChar
#  define termLiteralNextCharp	&termLiteralNextChar
#  define termStartCharp	&termStartChar
#  define termStopCharp		&termStopChar
#  define termForw1Charp	&termForw1Char
#  define termForw2Charp	&termForw2Char


/* Ring buffer structures which are shared */

extern Ring
    netoring,
    netiring,
    ttyoring,
    ttyiring;

/* Tn3270 section */
#if	defined(TN3270)

extern int
    HaveInput,		/* Whether an asynchronous I/O indication came in */
    noasynchtty,	/* Don't do signals on I/O (SIGURG, SIGIO) */
    noasynchnet,	/* Don't do signals on I/O (SIGURG, SIGIO) */
    sigiocount,		/* Count of SIGIO receptions */
    shell_active;	/* Subshell is active */

extern char
    *Ibackp,		/* Oldest byte of 3270 data */
    Ibuf[],		/* 3270 buffer */
    *Ifrontp,		/* Where next 3270 byte goes */
    tline[],
    *transcom;		/* Transparent command */

extern int
    settranscom();

extern void
    inputAvailable();
#endif	/* defined(TN3270) */
