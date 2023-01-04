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
 *	@(#)$RCSfile: ttydefaults.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 18:51:50 $
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/*
 * System wide defaults for terminal state.
 * Included by sys/termios.h.
 */
#ifndef _SYS_TTYDEFAULTS_
#define	_SYS_TTYDEFAULTS_

/*
 * Defaults on "first" open.
 */
#ifdef _KERNEL
#define	TTYDEF_IFLAG	(BRKINT | ICRNL | IXON)
#else
#define	TTYDEF_IFLAG	(BRKINT | ICRNL | IMAXBEL | IXON | IXANY)
#endif
#define TTYDEF_OFLAG	(OPOST | ONLCR)
#ifdef _KERNEL
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG)
#else
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG | IEXTEN | ECHOE|ECHOKE|ECHOCTL)
#endif
#define TTYDEF_CFLAG	(CREAD | CS8 | HUPCL)
#define TTYDEF_SPEED	(B9600)

/*
 * Control Character Defaults
 */
#define CTRL(x)		((x)&037)
#define CBELL		((unsigned)'\007')	/* ASCII BELL control */
#define CESC		((unsigned)'\\')	/* back-slash */
#define CFORM		((unsigned)'\014')	/* ^L */
#define CNUL		((unsigned)'\0')	/* null char */
#define CVT		((unsigned)'\013')	/* ^K */
#define CEOL		((unsigned)'\377')	/* XXX avoid _POSIX_VDISABLE */
#define CDEL		CEOL			/* delete */
#define CSTATUS		((unsigned)'\377')	/* XXX avoid _POSIX_VDISABLE */
#define CEOF		((unsigned)'\004')	/* ^D */
#define CEOT		CEOF
#define CERASE		((unsigned)'\010')	/* ^H */
#define CINTR		((unsigned)'\003')	/* ^C */
#define CKILL		((unsigned)'\025')	/* ^U */
#define CMIN		1
#define CQUIT		((unsigned)'\034')	/* ^\ */
#define CSUSP		((unsigned)'\032')	/* ^Z */
#define CTIME		0
#define CDSUSP		((unsigned)'\031')	/* ^Y */
#define CSTART		((unsigned)'\021')	/* ^Q */
#define CSTOP		((unsigned)'\023')	/* ^S */
#define CLNEXT		((unsigned)'\026')	/* ^V */
#define CDISCARD	((unsigned)'\017')	/* ^O */
#define CWERASE		((unsigned)'\027')	/* ^W */
#define CREPRINT	((unsigned)'\022')	/* ^R */
/* compat */
#define	CBRK		CEOL
#define CRPRNT		CREPRINT
#define CFLUSHO		CDISCARD
#define CFLUSH		CDISCARD

/* PROTECTED INCLUSION ENDS HERE */
#endif /* _SYS_TTYDEFAULTS_ */

/*
 * #define TTYDEFCHARS to include an array of default control characters.
 */
#ifdef TTYDEFCHARS
#ifndef _KERNEL
cc_t	ttydefchars[NCCS] = {
	CEOF,	CEOL,	CEOL,	CERASE, CWERASE, CKILL,
	CREPRINT, _POSIX_VDISABLE, CINTR, CQUIT, CSUSP,	CDSUSP,
	CSTART,	CSTOP,	CLNEXT, CDISCARD, CMIN,	CTIME,
	CSTATUS, _POSIX_VDISABLE
};
#else
cc_t	ttydefchars[NCCS] = {
	CEOF,	CEOL, _POSIX_VDISABLE, CERASE, _POSIX_VDISABLE, CKILL,
	_POSIX_VDISABLE, _POSIX_VDISABLE, CINTR, CQUIT, CSUSP, _POSIX_VDISABLE,
	CSTART, CSTOP, _POSIX_VDISABLE, _POSIX_VDISABLE, CMIN,	CTIME,
	_POSIX_VDISABLE, _POSIX_VDISABLE
};
#endif /* _KERNEL */
#undef TTYDEFCHARS
#endif /* TTYDEFCHARS */
