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
static char	*sccsid = "@(#)$RCSfile: timer.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:26:52 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

*/
/* 
 * COMPONENT_NAME: TCPIP timer.c
 * 
 * FUNCTIONS: hup, timer 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1988 Regents of the University of California.
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
 */
/*
#ifndef lint
static char sccsid[] = "timer.c	5.9 (Berkeley) 6/1/90";
#endif  not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

int	faketime;

/*
 * Timer routine.  Performs routing information supply
 * duties and manages timers on routing table entries.
 * Management of the RTS_CHANGED bit assumes that we broadcast
 * each time called.
 */
void
timer()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1, timetobroadcast;
	extern int externalinterfaces;

	(void) gettimeofday(&now, (struct timezone *)NULL);
	faketime += TIMER_RATE;
	if (lookforinterfaces && (faketime % CHECK_INTERVAL) == 0)
		ifinit();
	timetobroadcast = supplier && (faketime % SUPPLY_INTERVAL) == 0;
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			/*
			 * We don't advance time on a routing entry for
			 * a passive gateway, or any interface if we're
			 * not acting as supplier.
			 */
			if (!(rt->rt_state & RTS_PASSIVE) &&
			    (supplier || !(rt->rt_state & RTS_INTERFACE)))
				rt->rt_timer += TIMER_RATE;
			if (rt->rt_timer >= GARBAGE_TIME) {
				rt = rt->rt_back;
				rtdelete(rt->rt_forw);
				continue;
			}
			if (rt->rt_timer >= EXPIRE_TIME &&
			    rt->rt_metric < HOPCNT_INFINITY)
				rtchange(rt, &rt->rt_router, HOPCNT_INFINITY);
			rt->rt_state &= ~RTS_CHANGED;
		}
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (timetobroadcast) {
		toall(supply, 0, (struct interface *)NULL);
		lastbcast = now;
		lastfullupdate = now;
		needupdate = 0;		/* cancel any pending dynamic update */
		nextbcast.tv_sec = 0;
	}
}

/*
 * On hangup, let everyone know we're going away.
 */
void
hup()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1;

	if (supplier) {
again:
		for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
			rt = rh->rt_forw;
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw)
				rt->rt_metric = HOPCNT_INFINITY;
		}
		if (doinghost) {
			doinghost = 0;
			base = nethash;
			goto again;
		}
		toall(supply, 0, (struct interface *)NULL);
	}
	exit(1);
}
