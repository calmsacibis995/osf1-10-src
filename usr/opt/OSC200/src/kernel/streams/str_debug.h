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
 * @(#)$RCSfile: str_debug.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 20:09:27 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 */

#ifndef	_STR_DEBUG_H
#define	_STR_DEBUG_H

#undef	DB_CHECK_LOCK		/* has not been updated for QUEUE synchr. */

/*
 *	Debug areas
 */

#define DB_SCHED	 1
#define DB_SYS		 2
#define DB_LINK    	 3
#define DB_PUTMSG	 4
#define DB_GETMSG	 5
#define DB_PUSHPOP	 6
#define DB_ALLOC   	 7
#define DB_CANPUT	 8
#define DB_ECHO		 9
#define DB_DBG_TLI	10
#define DB_SIG		11
#define DB_MP		12
#define DB_SYNC		13
#define	DB_CTTY		14
#define DB_FUNC		15

#define DB_LAST		16	/* must be last in sequence! */

/*
 *	System call indentifications
 */

#define	SYSCALL_OPEN	1
#define SYSCALL_CLOSE	2
#define SYSCALL_READ	3
#define SYSCALL_WRITE	4
#define SYSCALL_IOCTL	5

/*
 *	The following
 *		- provides the external declarations (  DEBUG + _NO_PROTO )
 *	OR	- provides the right prototypes ( DEBUG + ANSI C )
 *	OR	- makes all debug calls disappear from the code ( NO DEBUG )
 *
 *	Some debug facilities are only appropriate for certain versions,
 *	and thus may have further configuration controls.
 */

/*
 *	printf facility - works in every version
 */

#if	STREAMS_DEBUG

#define STR_DEBUG(stmt)	stmt

extern	void	DB_init();
extern	void	DB0(int, int, char *);
extern	void	DB1(int, int, char *, caddr_t);
extern	void	DB2(int, int, char *, caddr_t, caddr_t);
extern	void	DB3(int, int, char *, caddr_t, caddr_t, caddr_t);
extern	void	DB4(int, int, char *, caddr_t, caddr_t, caddr_t, caddr_t);
extern	void	DB5(int, int, char *, caddr_t, caddr_t, caddr_t, caddr_t, caddr_t);
extern	void	DB_show_stream(STHP);

#else	/* !STREAMS_DEBUG */

#define STR_DEBUG(stmt)	/* null */

#define	DB_init()
#define DB0(area, level, fmt)
#define DB1(area, level, fmt, a)
#define DB2(area, level, fmt, a, b)
#define DB3(area, level, fmt, a, b, c)
#define DB4(area, level, fmt, a, b, c, d)
#define DB5(area, level, fmt, a, b, c, d, e)
#define	DB_show_stream(sth)

#endif	/* !STREAMS_DEBUG */

/*
 * Function call monitoring
 */

#if	STREAMS_DEBUG
extern	void	STREAMS_ENTER_FUNC(int (*)(), int, int, int, int);
extern	void	STREAMS_LEAVE_FUNC(int (*)(), int);
extern	void	REPORT_FUNC();
#define	ENTER_FUNC(func, a, b, c, d) \
	STREAMS_ENTER_FUNC((int(*)())(func),(int)(a),(int)(b),(int)(c),(int)(d))
#define	LEAVE_FUNC(func, retval) \
	STREAMS_LEAVE_FUNC((int(*)())(func),(int)(retval))
#else
#define	ENTER_FUNC(func, a, b, c, d)
#define	LEAVE_FUNC(func, retval)
#define	REPORT_FUNC()
#endif

/*
 * STREAMS_CHECK package - works only for STREAMS_DEBUG
 */

#if	STREAMS_DEBUG && defined(DB_CHECK_LOCK)
#define	STREAMS_CHECK	1
#else
#define	STREAMS_CHECK	0
#endif

#if	STREAMS_CHECK
extern	void	DB_isopen(STHP);
extern	void	DB_isclosed(STHP);
extern	void	DB_check_streams(char *);
#else
#define	DB_isopen(sth)
#define	DB_isclosed(sth)
#define DB_check_streams(caller)
#endif
#endif
