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
static char	*sccsid = "@(#)$RCSfile: uipc_proto.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 23:52:41 $";
#endif 
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	uipc_proto.c	7.3 (Berkeley) 6/29/88
 *	Merged: uipc_proto.c	7.4 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

/*
 * Definitions of protocols supported in the UNIX domain.
 */

extern	struct domain unixdomain;		/* or at least forward */


#define UIPC_LOCK_FUNNEL (MACH && NETSYNC_SPL && NETSYNC_LOCKTEST)

#if	UIPC_LOCK_FUNNEL
/* Something of a demonstrator. */
#include "kern/lock.h"
static struct {
	lock_data_t	l;	/* the lock */
	int		c;	/* the depth */
} uipc_lock;
#endif

uipc_config()
{
#if	UIPC_LOCK_FUNNEL
	lock_init(&uipc_lock.l, 1);
	uipc_lock.c = 0;
#endif
	return domain_add(&unixdomain);
}

#if	NETSYNC_SPL

#if	MACH
#include "kern/parallel.h"
#endif

static void
uipc_sanity()
{
	panic("uipc unfunnel");
}

static void
uipc_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = uipc_sanity;
#if	!UIPC_LOCK_FUNNEL
	NETSPLX(dfp->object.spl);
	unix_release();
#else
	if (--uipc_lock.c < 0)
		panic("uipc_unfunnel");
	if (uipc_lock.c == 0)
		lock_clear_recursive(&uipc_lock.l);
	lock_done(&uipc_lock.l);
#endif
}

static void
uipc_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("uipc funnel");
	dfp->unfunnel = uipc_unfunnel;
#if	!UIPC_LOCK_FUNNEL
	unix_master();
	NETSPL(dfp->object.spl,net);
#else
	lock_write(&uipc_lock.l);
	if (++uipc_lock.c == 1)
		lock_set_recursive(&uipc_lock.l);
#endif
}

#if	!UIPC_LOCK_FUNNEL
/* No "force unfunnel" op required with unix_master/spl */
#define	uipc_funfrc	0
#else
static void
uipc_funfrc(dfp)
	struct domain_funnel *dfp;
{
	/* Same function used for both unfunnel and refunnel */
	if (dfp->unfunnel) {
		/* Restore lock(s) to same depth */
		lock_write(&uipc_lock.l);
		lock_set_recursive(&uipc_lock.l);
		while (++uipc_lock.c != dfp->object.spl)
			lock_write(&uipc_lock.l);
	} else {
		/* Release lock(s), saving depth in spl */
		if (uipc_lock.c <= 0)
			panic("uipc_funfrc");
		dfp->unfunnel = uipc_funfrc;
		dfp->object.spl = uipc_lock.c;
		do {
			if (--uipc_lock.c == 0)
				lock_clear_recursive(&uipc_lock.l);
			lock_done(&uipc_lock.l);
		} while (uipc_lock.c);
	}
}
#endif

#else
#define uipc_funnel	0
#define uipc_funfrc	0
#endif

CONST struct protosw unixsw[] = {
{ SOCK_STREAM,	&unixdomain,	0,	PR_CONNREQUIRED|PR_WANTRCVD|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  uipc_init,	0,		0,		0
},
{ SOCK_DGRAM,	&unixdomain,	0,		PR_ATOMIC|PR_ADDR|PR_RIGHTS,
  0,		0,		0,		0,
  uipc_usrreq,
  0,		0,		0,		0
},
{ 0,		0,		0,		0,
  (void (*)())raw_input, 0,	raw_ctlinput,	0,
  raw_usrreq,
  0,		0,		0,		0
}
};

struct domain unixdomain =
    { AF_UNIX, "unix", 0, unp_externalize, unp_dispose,
      unixsw, &unixsw[sizeof(unixsw)/sizeof(unixsw[0])],
      0, 0, uipc_funnel, uipc_funfrc };
