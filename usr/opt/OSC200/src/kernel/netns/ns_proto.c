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
static char	*sccsid = "@(#)$RCSfile: ns_proto.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:49:40 $";
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
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *	Base:	ns_proto.c	7.4 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "netns/ns.h"
#include "netns/ns_config.h"

#if	MACH
#include "sys/sysconfig.h"
#include "kern/parallel.h"
#endif

/*
 * NS protocol family: IDP, ERR, PE, SPP, ROUTE.
 */

extern	struct domain nsdomain;

#if	!MACH
ns_config()
{
	return domain_add(&nsdomain);
}
#else
ns_config(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t	op;
	ns_config_t *	indata;
	size_t		indatalen;
	ns_config_t *	outdata;
	size_t		outdatalen;
{
	int error;

	switch (op) {
	case SYSCONFIG_CONFIGURE:
		if (indata != NULL && indatalen == sizeof(ns_config_t) &&
		    indata->version == NS_CONFIG_VERSION_1) {
			/* TODO - Install input parameters, if any */
		}
		if ((error = domain_add(&nsdomain)) == 0) {
			if (outdata != NULL &&
			    outdatalen == sizeof(ns_config_t)) {
				/* TODO - Return some stuff */
			}
		}
		break;
	case SYSCONFIG_UNCONFIGURE:
		/* Will not work, for various reasons, but here anyway. */
		error = domain_del(&nsdomain);
		break;
	default:
		error = EINVAL;
		break;
	}
	return (error);
}
#endif

/*
 * XNS protocols are unmodified for parallelization, and are
 * thus unconditionally funneled with raised spl by the
 * socket layer from above and the netisr's from below.
 */

static void
ns_sanity()
{
	panic("ns unfunnel");
}

static void
ns_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = ns_sanity;
	NETSPLX(dfp->object.spl);
	unix_release();
}

static void
ns_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("ns funnel");
	dfp->unfunnel = ns_unfunnel;
	unix_master();
	NETSPL(dfp->object.spl,net);
}

/* No "force unfunnel" op required with unix_master/spl */

#define V(x)	(void (*)())x

struct protosw nssw[] = {
{ 0,		&nsdomain,	0,		0,
  0,		idp_output,	0,		0,
  0,
  V(ns_init),	0,		0,		0
},
{ SOCK_DGRAM,	&nsdomain,	0,		PR_ATOMIC|PR_ADDR,
  0,		0,		V(idp_ctlinput),idp_ctloutput,
  idp_usrreq,
  0,		0,		0,		0
},
{ SOCK_STREAM,	&nsdomain,	NSPROTO_SPP,	PR_CONNREQUIRED|PR_WANTRCVD,
  V(spp_input),	0,		V(spp_ctlinput),spp_ctloutput,
  spp_usrreq,
  V(spp_init),	V(spp_fasttimo),V(spp_slowtimo),0
},
{ SOCK_SEQPACKET,&nsdomain,	NSPROTO_SPP,	PR_CONNREQUIRED|PR_WANTRCVD|PR_ATOMIC,
  V(spp_input),	0,		V(spp_ctlinput),spp_ctloutput,
  spp_usrreq_sp,
  0,		0,		0,		0
},
{ SOCK_RAW,	&nsdomain,	NSPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  V(idp_input),	idp_output,	0,		idp_ctloutput,
  idp_raw_usrreq,
  0,		0,		0,		0
},
{ SOCK_RAW,	&nsdomain,	NSPROTO_ERROR,	PR_ATOMIC|PR_ADDR,
  V(idp_ctlinput),idp_output,	0,		idp_ctloutput,
  idp_raw_usrreq,
  0,		0,		0,		0
}
};

struct domain nsdomain =
    { AF_NS, "network systems", 0, 0, 0, 
      nssw, &nssw[sizeof(nssw)/sizeof(nssw[0])],
      0, 0, ns_funnel, 0 };

