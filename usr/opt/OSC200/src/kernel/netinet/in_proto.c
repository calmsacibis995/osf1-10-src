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
static char	*sccsid = "@(#)$RCSfile: in_proto.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/05/16 21:20:01 $";
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
 *	Base:	in_proto.c	7.4 (Berkeley) 4/22/89
 *	Merged:	in_proto.c	7.5 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/inet_config.h"

#if	MACH
#include "sys/sysconfig.h"
#if	NETSYNC_SPL
#include "kern/parallel.h"
#endif
#endif

/* NSIP encapsulation option check */
#ifndef	NS
#include <ns.h>
#endif
#if	NS > 0
#include "netns/proto_ns.h"
#endif

/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */

extern	struct domain inetdomain;

#if	!MACH
inet_config()
{
	return domain_add(&inetdomain);
}
#else
inet_config(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t	op;
	inet_config_t *	indata;
	size_t		indatalen;
	inet_config_t *	outdata;
	size_t		outdatalen;
{
	int error;

	switch (op) {
	case SYSCONFIG_CONFIGURE:
		if (indata != NULL && indatalen == sizeof(inet_config_t) &&
		    indata->version == INET_CONFIG_VERSION_1) {
			/* TODO - Install input parameters, if any */
		}
		if ((error = domain_add(&inetdomain)) == 0) {
			if (outdata != NULL &&
			    outdatalen == sizeof(inet_config_t)) {
				/* TODO - Return some stuff */
			}
		}
		break;
	case SYSCONFIG_UNCONFIGURE:
		/* Will not work, for various reasons, but here anyway. */
		error = domain_del(&inetdomain);
		break;
	default:
		error = EINVAL;
		break;
	}
	return (error);
}
#endif

#if	NETSYNC_SPL
static void
inet_sanity()
{
	panic("inet unfunnel");
}

static void
inet_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = inet_sanity;
	NETSPLX(dfp->object.spl);
	unix_release();
}

static void
inet_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("inet funnel");
	dfp->unfunnel = inet_unfunnel;
	unix_master();
	NETSPL(dfp->object.spl,net);
}

/* No "force unfunnel" op required with unix_master/spl */

#else
#define inet_funnel	0
#endif

CONST struct protosw inetsw[] = {
{ 0,		&inetdomain,	0,		0,
  0,		ip_output,	0,		0,
  0,
  ip_init,	0,		ip_slowtimo,	ip_drain
},
{ SOCK_DGRAM,	&inetdomain,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR,
  udp_input,	0,		udp_ctlinput,	ip_ctloutput,
  udp_usrreq,
  udp_init,	0,		0,		0,
},
{ SOCK_STREAM,	&inetdomain,	IPPROTO_TCP,	PR_CONNREQUIRED|PR_WANTRCVD,
  tcp_input,	0,		tcp_ctlinput,	tcp_ctloutput,
  tcp_usrreq,
  tcp_init,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  rip_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,
  0,		0,		0,		0
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_ICMP,	PR_ATOMIC|PR_ADDR,
  icmp_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,
  0,		0,		0,		0
},
{ SOCK_RAW,	&inetdomain,	IPPROTO_IGMP,	PR_ATOMIC|PR_ADDR,
  igmp_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,
  igmp_init,	igmp_fasttimo,	0,		0
},
#if	(NS > 0) && NETSYNC_SPL
#define V(x)	(void (*)())x
{ SOCK_RAW,	&inetdomain,	IPPROTO_IDP,	PR_ATOMIC|PR_ADDR,
  V(idpip_input),rip_output,	V(nsip_ctlinput),0,
  rip_usrreq,
  0,		0,		0,		0
},
#endif
	/* raw wildcard */
{ SOCK_RAW,	&inetdomain,	0,		PR_ATOMIC|PR_ADDR,
  rip_input,	rip_output,	0,		rip_ctloutput,
  rip_usrreq,
  0,		0,		0,		0
}
};

struct domain inetdomain =
    { AF_INET, "internet", 0, 0, 0, 
      inetsw, &inetsw[sizeof(inetsw)/sizeof(inetsw[0])],
      0, 0, inet_funnel, 0 };
