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
static char rcsid[] = "@(#)$RCSfile: signum.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/03 03:31:25 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: signum,sigprt
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * signum.c    1.5  com/cmd/cntl,3.1,9021 2/16/90 13:36:05
 */
#include <ctype.h>
#include <sys/signal.h>


/*
 * Table of signal names associated with signal codes.
 */

static char *SGcodes[NSIG] = {
	"NULL",		"HUP",		"INT",		"QUIT", /* 0-3   */
	"ILL",		"TRAP",		"IOT",		"EMT",	/* 4-7   */
	"FPE",		"KILL",		"BUS",		"SEGV", /* 8-11  */
	"SYS",		"PIPE",		"ALRM",		"TERM", /* 12-15 */
	"URG",		"STOP",		"TSTP",		"CONT", /* 16-19 */
	"CHLD",		"TTIN",		"TTOU",		"IO",   /* 20-23 */
#ifdef _AIX
	"XCPU",		"XFSZ",		(char *)0,	"MSG",
	"WINCH",	"PWR",		"USR1",		"USR2",
	"PROF",		"DANGER",	"VTALRM",	"MIGRATE",
	"PRE",		(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	(char *)0,	(char *)0,	(char *)0,	(char *)0,
	"GRANT",	"RETRACT",	"SOUND",	"SAK"

#else
	"XCPU",		"XFSZ",		"VTALRM",	"PROF", /* 24-27 */
	"WINCH",	"INFO",	 	"USR1",		"USR2"  /* 28-31 */
#endif
};


/*
 * NAME:  signum
 * FUNCTION: signum will convert a signal name to a signal number.
 *    if signal is a number that signum will convert it to an integer.
 */
int signum(char *signal)
{
	int i;
	char *s;

	if (signal == NULL || *signal == '\0')
		return (-1);

	if (isdigit(signal[0])) {
		i = atoi(signal);
		if ((i >= 0) && (i < NSIG))
			return(i);
		else {
			i &= 0177;
			if ((i >= 0) && (i < NSIG))
				return(i);
		}
		return(-1);
	}
	for (i=0; i < strlen(signal); ++i) 
		signal[i]=toupper((int)signal[i]);
	/* strip SIG off of string */
	if (signal[0] == 'S' && signal[1] == 'I' && signal [2] == 'G')
		s = signal+3;
	else
		s = signal;
	/* compare string to list of signals in SGcodes */
	for (i=0; i < NSIG; i++)
		if (!strcmp(s,SGcodes[i])) {
			return(i);		
		}
	return(-1);
}

/*
 * NAME: sigprt
 * FUNCTION: sigprt will print out a list of signal names without the prefix SIG
 *	if called with an argument of -1.  If called with a signal number, only
 *	one signal name will be printed out.
 */
void sigprt(int signo)
{
	int i,low,high;

	if (signo == -1) {
	  for (i = 0; i < NSIG-1; i++)
	    printf("%s ", SGcodes[i]);

	  printf("%s\n", SGcodes[NSIG-1]);
	} else
	  printf("%s\n", SGcodes[signo]);
}

