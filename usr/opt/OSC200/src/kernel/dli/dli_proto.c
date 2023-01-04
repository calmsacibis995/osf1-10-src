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
#ifndef	lint
static char *sccsid = "@(#)$RCSfile: dli_proto.c,v $ $Revision: 4.4.10.4 $ (DEC) $Date: 1993/10/08 13:51:22 $";
#endif

/*
 * Program dli_proto.c,  Module DLI 
 *
 * Copyright (C) 1985 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 * 2.00 18-Apr-1986
 *		DECnet-Ultrix	V2.0
 *
 * 5.00 09-Oct-1989
 *		DECnet-Ultrix	V5.0
 *
 * Added sysid and point-to-point support
 *
 */


#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/if_llc.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <dli/dli_var.h>
#include <dli.h>

#if DLI > 0

#if	MACH
#include <sys/sysconfig.h>
#include <kern/parallel.h>
#endif

#if defined(NETMAN) && defined(DLIMGT)
#define _DLI_MANAGEMENT 1
#endif

#ifdef _DLI_MANAGEMENT
extern int dlimgt_output();

extern void csmacd_init(),csmacd_ctlinput();
extern int csmacd_ctloutput();

extern void fddi_init(),fddi_ctlinput();
extern int fddi_ctloutput();

extern void trn_init(),trn_ctlinput();
extern int trn_ctloutput();

extern void dloop_init();
extern int dloop_ctloutput(),dloop_output();

#endif /* _DLI_MANAGEMENT */

/*
 * Definitions of protocols supported in the DLI domain.
 */
int dli_ifq_maxlen = 128;
int dli_mustberoot = 1;
struct dlimgt_routines *dlimgtsw = NULL;
struct dli_ifnet *dli_dlifs = NULL;

dli_config()
{
    return domain_add(&dlidomain);
}

extern int dli_usrreq(), dli_ctloutput();
extern void dli_init(), dli_slowtimo();
extern void dli_funnel();

struct protosw dlisw[] = {
    {	SOCK_DGRAM,	&dlidomain,	DLPROTO_DLI,	PR_ATOMIC|PR_ADDR|PR_READZEROLEN,
	0,		0,		0,		dli_ctloutput,
	dli_usrreq,	dli_init,	0,		dli_slowtimo,
	0,
    },
#ifdef _DLI_MANAGEMENT
#ifndef _DLIMGT_NO_CSMACD_
    { SOCK_DGRAM,     &dlidomain,       DLPROTO_CSMACD,  PR_ATOMIC | PR_ADDR,
      0,              dlimgt_output,    csmacd_ctlinput, csmacd_ctloutput,
      0,              csmacd_init,      0,               0,
      0
    },
#endif
#ifndef _DLIMGT_NO_FDDI
    { SOCK_DGRAM,     &dlidomain,       DLPROTO_FDDI,	 PR_ATOMIC | PR_ADDR,
      0,              dlimgt_output,    fddi_ctlinput,   fddi_ctloutput,
      0,              fddi_init,        0,               0,
      0
    },
#endif
#ifndef _DLIMGT_NO_TRN
    { SOCK_DGRAM,     &dlidomain,       DLPROTO_TRN,	 PR_ATOMIC | PR_ADDR,
      0,              dlimgt_output,    trn_ctlinput,    trn_ctloutput,
      0,              trn_init,         0,               0,
      0
    },
#endif
    { SOCK_DGRAM,     &dlidomain,       DLPROTO_LOOP,	 PR_ATOMIC | PR_ADDR,
      0,              dloop_output,     0,               dloop_ctloutput,
      0,              dloop_init,       0,               0,
      0
    },
#endif /* _DLI_MANAGEMENT_ */
};


/*
 * DLI has not been modified for parallelization, and is
 * thus unconditionally funneled with raised spl by the
 * socket layer from above and the netisr's from below.
 */

static void
dli_sanity()
{
	panic("dli unfunnel");
}

static void
dli_unfunnel(dfp)
	struct domain_funnel *dfp;
{
	dfp->unfunnel = dli_sanity;
	NETSPLX(dfp->object.spl);
	unix_release();
}

/* this routine is also called by the NETMAN subset */
void
dli_funnel(dfp)
	struct domain_funnel *dfp;
{
	if (dfp->unfunnel)
		panic("dli funnel");
	dfp->unfunnel = dli_unfunnel;
	unix_master();
	NETSPL(dfp->object.spl,net);
}

struct domain dlidomain = {
    AF_DLI,  "DLI", 0, 0, 0,
    dlisw, &dlisw[sizeof(dlisw)/sizeof(dlisw[0])],
    0, 0, dli_funnel, 0 };

#else /* #if DLI == 0 */

/* this routine must exist whether DLI is in kernel or not, so that
 * netinit() can call it, since netisr.c is not recompiled for different
 * flavors of kernel
 */
dli_config()
{
    return 0;
}

#endif /* DLI */

