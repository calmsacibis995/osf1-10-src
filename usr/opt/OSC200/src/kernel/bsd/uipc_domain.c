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
static char	*sccsid = "@(#)$RCSfile: uipc_domain.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/07/31 18:48:40 $";
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
 *	Base:	@(#)uipc_domain.c	3.1 (Berkeley) 2/26/91
 *	Merged: uipc_domain.c	7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/kernel.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/errno.h"

#define HDRALIGN(x)	(((x)+sizeof(u_long)-1) & ~(sizeof(u_long)-1))

LOCK_ASSERTL_DECL

struct domain	*domains;
#if	NETSYNC_LOCK
lock_data_t	domain_lock;
#endif

void
domaininit()
{

	domains = 0;
	DOMAIN_LOCKINIT();
	if (max_linkhdr == 0)
		max_linkhdr = 16;
	max_protohdr = 0;
	max_hdr = max_linkhdr + max_protohdr;
	max_datalen = MHLEN - max_hdr;
#if	!NETISR_THREAD
	pffasttimo();
	pfslowtimo();
#else
	net_threadstart(pffasttimo, 10);
	net_threadstart(pfslowtimo, -1);
#endif
}

domain_add(dp)
	register struct domain *dp;
{
	register struct domain *tdp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_WRITE_LOCK();
	for (tdp = domains; tdp; tdp = tdp->dom_next)
		if (tdp->dom_family == dp->dom_family) {
			DOMAIN_WRITE_UNLOCK();
			return EEXIST;
		}
	DOMAINRC_LOCKINIT(dp);
	dp->dom_refcnt = 0;
	dp->dom_next = domains;
	domains = dp;
	DOMAIN_WRITE_UNLOCK();

	DOMAIN_FUNNEL(dp, f);
	if (dp->dom_init)
		(*dp->dom_init)();
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
		if (pr->pr_init)
			(*pr->pr_init)();
	DOMAIN_UNFUNNEL(f);

	DOMAIN_WRITE_LOCK();
	max_linkhdr = HDRALIGN(max_linkhdr);
	max_protohdr = HDRALIGN(max_protohdr);
	max_hdr = max_linkhdr + max_protohdr;
	max_datalen = MHLEN - max_hdr;
	if (max_datalen < 0)
		panic("domain_add max_hdr");
	DOMAIN_WRITE_UNLOCK();
	return 0;
}

domain_del(dp)
	struct domain *dp;
{
	int error = 0;
	DOMAIN_LOCK_DECL()

	DOMAIN_WRITE_LOCK();
	DOMAINRC_LOCK(dp);
	if (dp->dom_refcnt == 0) {
		if (dp == domains)
			domains = dp->dom_next;
		else {
			register struct domain *tdp;
			for (tdp = domains; tdp; tdp = tdp->dom_next)
				if (tdp->dom_next == dp) {
					tdp->dom_next = dp->dom_next;
					break;
				}
			if (tdp == NULL)
				error = ENOENT;
		}
	} else
		error = EADDRINUSE;
	DOMAINRC_UNLOCK(dp);
	DOMAIN_WRITE_UNLOCK();
	return error;
}

struct protosw *
pffindtype(family, type)
	int family, type;
{
	register struct domain *dp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	DOMAIN_READ_UNLOCK();
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
		if (pr->pr_type && pr->pr_type == type) {
			DOMAINRC_REF(dp);
			DOMAIN_READ_UNLOCK();
			return (pr);
		}
	DOMAIN_READ_UNLOCK();
	return (0);
}

struct protosw *
pffindproto(family, protocol, type)
	int family, protocol, type;
{
	register struct domain *dp;
	register struct protosw *pr;
	struct protosw *maybe = 0;
	DOMAIN_LOCK_DECL()

	if (family == 0)
		return (0);
	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	DOMAIN_READ_UNLOCK();
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++) {
		if ((pr->pr_protocol == protocol) && (pr->pr_type == type)) {
			maybe = pr;
			break;
		}
		if (type == SOCK_RAW && pr->pr_type == SOCK_RAW &&
		    pr->pr_protocol == 0 && maybe == (struct protosw *)0)
			maybe = pr;
	}
	if (maybe)
		DOMAINRC_REF(dp);
	DOMAIN_READ_UNLOCK();
	return (maybe);
}

void
pfctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	register struct domain *dp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_ctlinput)
				(*pr->pr_ctlinput)(cmd, sa, (caddr_t) 0);
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
}

void
pfreclaim()
{
	register struct domain *dp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_drain)
				(*pr->pr_drain)();
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
}

pfslowtimo()
{
	register struct domain *dp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_slowtimo)
				(*pr->pr_slowtimo)();
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
#if	!NETISR_THREAD
	timeout(pfslowtimo, (caddr_t)0, hz/PR_SLOWHZ);
#else
	return (hz/PR_SLOWHZ);
#endif
}

pffasttimo()
{
	register struct domain *dp;
	register struct protosw *pr;
	DOMAIN_LOCK_DECL()
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_READ_LOCK();
	for (dp = domains; dp; dp = dp->dom_next) {
		DOMAIN_FUNNEL(dp, f);
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_fasttimo)
				(*pr->pr_fasttimo)();
		DOMAIN_UNFUNNEL(f);
	}
	DOMAIN_READ_UNLOCK();
#if	!NETISR_THREAD
	timeout(pffasttimo, (caddr_t)0, hz/PR_FASTHZ);
#else
	return (hz/PR_FASTHZ);
#endif
}
