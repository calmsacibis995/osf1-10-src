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
static char *rcsid = "@(#)$RCSfile: str_debug.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 20:09:19 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <sys/stat.h>
#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

#if	STREAMS_DEBUG

/*
 *	Initialization hook.
 *
 *	In case any of these debug packages need intialization.
 */
void
DB_init()
{
}

/*
 *	Configurable debug printf's
 *
 *	The idea is that messages introduced into the code should
 *	disappear dynamically, and perhaps on a per-area base.
 *	Therefor we have a number of global variables (patchable
 *	with the debugger) which control the debug output globally
 *	of for a certain area.
 *
 *	Each debug printf in the code (using the calls DB0() - DB5(),
 *	depending of the number of "%" arguments) has an area and
 *	a level assigned to it. The message gets printed only if the
 *	level <= min(global level, area level). That is, the lower
 *	the number the more urgent the message, and the
 *	global level overrides local levels (see DB_is_on).
 *
 *	The reason for the six (instead of one) DB?() procedures is
 *	the fact that we want to make even the calls themselves
 *	disappear from the code, if STREAMS_DEBUG is not defined.
 *	And the C-Preprocessor doesn't know about varargs.
 */

int	DB_level	= 1;
int	DB_sched	= 1;
int	DB_sys		= 1;
int	DB_link		= 1;
int	DB_putmsg	= 1;
int	DB_getmsg	= 1;
int	DB_pushpop	= 1;
int	DB_alloc	= 1;
int	DB_canput	= 1;
int	DB_echo		= 1;
int	DB_dbg_tli	= 1;
int	DB_sig		= 1;
int	DB_mp		= 1;
int	DB_sync		= 1;
int	DB_ctty		= 1;
int	DB_func		= 1;

int
DB_is_on(area, level)
	int	area;
	int	level;
{
	if (level <= DB_level)	return 1;

	switch (area) {
	case DB_SCHED:		return level <= DB_sched;
	case DB_SYS:		return level <= DB_sys;
	case DB_LINK:		return level <= DB_link;
	case DB_PUTMSG:		return level <= DB_putmsg;
	case DB_GETMSG:		return level <= DB_getmsg;
	case DB_PUSHPOP:	return level <= DB_pushpop;
	case DB_ALLOC:		return level <= DB_alloc;
	case DB_CANPUT:		return level <= DB_canput;
	case DB_ECHO:		return level <= DB_echo;
	case DB_DBG_TLI:	return level <= DB_dbg_tli;
	case DB_SIG:		return level <= DB_sig;
	case DB_MP:		return level <= DB_mp;
	case DB_SYNC:		return level <= DB_sync;
	case DB_CTTY:		return level <= DB_ctty;
	case DB_FUNC:		return level <= DB_func;
	default:		return 1;	/* temp debugs... */
	}
}

void
DB0(area, level, fmt)
	int	area;
	int	level;
	char	*fmt;
{
	if ( DB_is_on(area, level) ) printf(fmt);
}

void
DB1(area, level, fmt, a)
	int	area;
	int	level;
	char	*fmt;
	caddr_t	a;
{
	if ( DB_is_on(area, level) ) printf(fmt, a);
}

void
DB2(area, level, fmt, a, b)
	int	area;
	int	level;
	char	*fmt;
	caddr_t	a, b;
{
	if ( DB_is_on(area, level) ) printf(fmt, a, b);
}

void
DB3(area, level, fmt, a, b, c)
	int	area;
	int	level;
	char	*fmt;
	caddr_t	a, b, c;
{
	if ( DB_is_on(area, level) ) printf(fmt, a, b, c);
}

void
DB4(area, level, fmt, a, b, c, d)
	int	area;
	int	level;
	char	*fmt;
	caddr_t	a, b, c, d;
{
	if ( DB_is_on(area, level) ) printf(fmt, a, b, c, d);
}

void
DB5(area, level, fmt, a, b, c, d, e)
	int	area;
	int	level;
	char	*fmt;
	caddr_t	a, b, c, d, e;
{
	if ( DB_is_on(area, level) ) printf(fmt, a, b, c, d, e);
}

/*
 * Function entry / exit monitoring.
 *
 * Very high performance impact, but nice potential for monitoring,
 * debugging and tracing.
 *
 * Note that tracing can be selected on the fly by patching the level
 * entry in the func_tab entry for the function under investigation.
 */

typedef	int	(*pfi_t)();

struct func_tab {
	pfi_t	ft_func;
	int	ft_level;
	int	ft_count;
	char	ft_name[16];
};

#ifdef	bufcall
#undef	bufcall
extern	int	bufcall();
#endif

struct func_tab func_tab[] = {
	
/*
 * str_scalls.c
 */
	{ (pfi_t)pse_open,		9,0,	"pse_open     " },
	{ (pfi_t)osr_open,		9,0,	"osr_open     " },
	{ (pfi_t)osr_reopen,		9,0,	"osr_reopen   " },
	{ (pfi_t)pse_close,		9,0,	"pse_close    " },
	{ (pfi_t)osr_close_subr,	9,0,	"osr_close_sub" },
	{ (pfi_t)pse_read,		9,0,	"pse_read     " },
	{ (pfi_t)pse_write,		9,0,	"pse_write    " },
	{ (pfi_t)pse_ioctl,		9,0,	"pse_ioctl    " },
	{ (pfi_t)pse_select,		9,0,	"pse_select   " },
/*
 * str_synch.c
 */
	{ (pfi_t)osr_run,		9,0,	"osr_run      " },
	{ (pfi_t)osrq_init,		9,0,	"osrq_init    " },
	{ (pfi_t)osrq_insert,		9,0,	"osrq_insert  " },
	{ (pfi_t)osrq_remove,		9,0,	"osrq_remove  " },
	{ (pfi_t)osr_sleep,		9,0,	"osr_sleep    " },
	{ (pfi_t)osrq_wakeup,		9,0,	"osrq_wakeup  " },
	{ (pfi_t)sqh_insert,		9,0,	"sqh_insert   " },
	{ (pfi_t)sqh_remove,		9,0,	"sqh_remove   " },
	{ (pfi_t)act_q_init,		9,0,	"act_q_init   " },
	{ (pfi_t)csq_run,		9,0,	"csq_run      " },
	{ (pfi_t)csq_protect,		9,0,	"csq_protect  " },
	{ (pfi_t)csq_which_q,		9,0,	"csq_which_q  " },
	{ (pfi_t)_csq_acquire,		9,0,	"csq_acquire  " },
	{ (pfi_t)_csq_release,		9,0,	"csq_release  " },
	{ (pfi_t)csq_turnover,		9,0,	"csq_turnover " },
	{ (pfi_t)csq_lateral,		9,0,	"csq_lateral  " },
	{ (pfi_t)mult_sqh_acquire,	9,0,	"mult_sqh_acqu" },
	{ (pfi_t)mult_sqh_release,	9,0,	"mult_sqh_rele" },
	{ (pfi_t)csq_newparent,		9,0,	"csq_newparent" },
	{ (pfi_t)csq_cleanup,		9,0,	"csq_cleanup  " },

/*
 * str_memory.c
 */
	{ (pfi_t)he_alloc,		9,0,	"he_alloc     " },
	{ (pfi_t)he_free,		9,0,	"he_free      " },
	{ (pfi_t)he_realloc,		9,0,	"he_realloc   " },
	{ (pfi_t)bufcall_configure,	9,0,	"bufcall_confi" },
	{ (pfi_t)bufcall_rsrv,		9,0,	"bufcall_rsrv " },
	{ (pfi_t)bufcall,		9,0,	"bufcall      " },
	{ (pfi_t)bufcall_init,		9,0,	"bufcall_init " },
	{ (pfi_t)allocb,		9,0,	"allocb       " },
	{ (pfi_t)freeb,			9,0,	"freeb        " },
/*
 * str_runq.c
 */
	{ (pfi_t)runq_init,		9,0,	"runq_init    " },
	{ (pfi_t)qenable,		9,0,	"qenable      " },
	{ (pfi_t)runq_run,		9,0,	"runq_run     " },
	{ (pfi_t)runq_sq_init,		9,0,	"runq_sq_init " },
	{ (pfi_t)runq_remove,		9,0,	"runq_remove  " },

/*
 * misc.
 */
	{ (pfi_t)osr_str,		7,0,	"osr_str      " },

	{(pfi_t) 0,			9,0,	"" }
};

struct func_tab unknown_func =
	{ (pfi_t) 0,			9,0,	"UNKNOWN      " };

struct func_tab *
ft_lookup(func)
	pfi_t func;
{
	struct func_tab *	ftp;

	for (ftp = func_tab; ftp->ft_func != (pfi_t) 0; ftp++)
		if (ftp->ft_func == func)
			return ftp;
	return &unknown_func;
}

void
STREAMS_ENTER_FUNC(func, p1, p2, p3, p4)
	pfi_t	func;
	int	p1, p2, p3, p4;
{
	struct func_tab * ftp = ft_lookup(func);

	DB5(
		DB_FUNC,
		ftp->ft_level,
		"ENTER %s(%x, %x, %x, %x)\n",
		ftp->ft_name,
		(caddr_t)p1, (caddr_t)p2, (caddr_t)p3, (caddr_t)p4
	);
	ftp->ft_count++;
}

void
STREAMS_LEAVE_FUNC(func, retval)
	pfi_t	func;
	int	retval;
{
	struct func_tab * ftp = ft_lookup(func);

	DB2(
		DB_FUNC,
		ftp->ft_level,
		"LEAVE %s(%x)\n",
		ftp->ft_name,
		(caddr_t)retval
	);
}

#define NDIGITS	5

void
int_to_str(number, buffer)
	int	number;
	char *	buffer;
{
	char *p = buffer + NDIGITS + 1;

	*--p = 0;

	while (p > buffer) {
		*--p = (number % 10) + '0';
		number /= 10;
	}
}

void
REPORT_FUNC()
{
	struct func_tab * ftp;
	int		count;
	char		count_buf[NDIGITS + 1];

	for (	ftp = func_tab, count = 1;
		ftp->ft_func != (pfi_t) 0;
		ftp++, count++ ) {

		int_to_str(ftp->ft_count, count_buf);
		DB2(
			DB_FUNC,
			5,
			"%s(%s) ",
			ftp->ft_name,
			count_buf
		);
		if ( count % 5 == 0 )
			DB0(DB_FUNC, 5, "\n");
	}
}

#ifdef	DB_CHECK_LOCK
/*
 *	Consistency check for stream heads
 *	(to be extended to other data structures...)
 *
 *	Currently used:	only for STREAMS_DEBUG version (not MP-safe).
 *
 *	Maintains a list of open streams, and checks whether all of them
 *	are released at strategic points (end of system calls). Does
 *	work only if there is only one process active using streams.
 *
 *	STRlist is also nice for debugging, to find out which stream
 *	heads are currently around.
 *
 *	Known problem: when flushing a M_PASSFP message, an implicit
 *	close happens which will report one stream as locked.
 */

#define MAXSTREAMS 100

STHP	STRlist[MAXSTREAMS];
STHP *	STRlast = STRlist;
int	DB_verbose = 0;


void
DB_isopen(sth)
	STHP	sth;
{
	STHP	*sth_p;

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		if (*sth_p == nil(STHP))
			break;
	}
	if (sth_p == STRlast)
		sth_p = STRlast++;

	*sth_p = sth;
}

void
DB_isclosed(sth)
	STHP	sth;
{
	STHP	*sth_p;

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		if (*sth_p == sth)
			break;
	}
	if (sth_p == STRlast)
	{
		printf("DB_isclosed: couldn't find pointer to closed stream\n");
		return;
	}
	*sth_p = nil(STHP);
	if (sth_p + 1 == STRlast)
		STRlast--;
}

void
DB_check_streams(caller)
	char *caller;
{
	STHP	*sth_p;
	void	DB_sth_check();

	for (sth_p = STRlist; sth_p < STRlast; sth_p++)
	{
		DB_sth_check(caller, *sth_p);
	}
}
	

void
DB_break()
{
}

void
DB_sth_check(caller, sth)
	char	*caller;
	STHP	sth;
{
	queue_t	*rq;
	queue_t	*wq;
	SQH	*rsqh;
	SQH	*wsqh;
	SQH	*rsqhp;
	SQH	*wsqhp;

	if (sth == nilp(STH))
		return;

	rq = sth->sth_rq;
	wq = sth->sth_wq;

	rsqh = &rq->q_sqh;
	wsqh = &wq->q_sqh;

	rsqhp = rsqh->sqh_parent;
	wsqhp = wsqh->sqh_parent;

	if (rsqhp->sqh_refcnt != 0 || DB_verbose) {
		printf(
"*** %s *** STH %x, DEV %x, RQ %x, RSQH %x, RSQHP %x, refcnt %d, owner %x\n",
			caller,
			sth,
			sth->sth_dev,
			rq,
			rsqh,
			rsqhp,
			rsqhp->sqh_refcnt,
			rsqhp->sqh_owner
		);
		DB_break();
	}

	if (wsqhp != rsqhp) {
		printf(
"*** %s *** STH %x, DEV %x, RQ %x WQ %x RSQHP %x != WSQHP %s !!!\n",
			caller,
			sth,
			sth->sth_dev,
			rq,
			wq,
			rsqhp,
			wsqhp
		);
		printf(
"*** %s *** STH %x, DEV %x, WQ %x, WSQH %x, WSQHP %x, refcnt %d, owner %x\n",
			caller,
			sth,
			sth->sth_dev,
			wq,
			wsqh,
			wsqhp,
			wsqhp->sqh_refcnt,
			wsqhp->sqh_owner
		);
	}
}
#endif	/* DB_CHECK_LOCK */

void
DB_showstream (sth)
	STHP	sth;
{
	queue_t	* q;

	for (q = sth->sth_wq; q; q = q->q_next)
	DB5(DB_PUSHPOP, 5, "name %s q 0x%x q_next 0x%x wrq 0x%x wrq_next 0x%x\n",
				q->q_qinfo->qi_minfo->mi_idname,
				(caddr_t)RD(q),
			   	(caddr_t)RD(q)->q_next,
				(caddr_t)q,
				(caddr_t)q->q_next);
}

#endif	/* STREAMS_DEBUG */
